# Nightfall — Autonomous Operation Envelope

Canonical contract for running the execution agent in an unattended loop. The
plan (`nightfall-plan-macos.md`) says *what* to build and *when to stop*. This
document says *how the loop runs between stops* so the maintainer can walk away.
Where this document and the plan conflict on loop mechanics, this document wins;
on scope/architecture/Q-decisions, the plan wins.

The maintainer decisions this document once flagged are now **resolved** and
recorded below (merge policy §2, run scope §7); they are also in
`docs/decisions-log.md`. The loop is driven by an external supervisor
(`scripts/supervise.sh`) that gates on `scripts/preflight.sh`, enforces the
budget ceiling and kill switch from outside the agent, and relays the agent's
in-repo state transitions to the maintainer — see §4 and §6.

---

## 1. Loop lifecycle (idempotent resume)

The agent must assume it will be killed mid-task and re-invoked later — over
months, repeatedly. Every invocation begins with reconciliation, never with
work:

1. Read `docs/current-phase.md`, `git status`, the open-PR list, and
   `docs/blockers/` for any unresolved blocker.
2. If an unresolved blocker exists: check it for an `## Adjudication` block
   (see §6). If none, **halt** — leave the blocker doc in place (the supervisor
   re-relays it) and do not start new work while a blocker is open; the loop is
   strictly sequential (§5).
3. If the working tree has uncommitted changes from a prior killed run, do not
   guess at them: reconcile against `current-phase.md`. If they form a coherent
   partial step, continue it; if ambiguous, commit them to a
   `wip/<phase>-recovered` branch and open a blocker doc rather than discard or
   merge them.
4. Confirm the working branch matches the phase recorded in `current-phase.md`.
5. Only then resume forward work.

**Never leave uncommitted state across an invocation boundary.** At the end of
every work cycle, commit and push WIP to the phase's working branch even if
incomplete, with a `WIP:` subject. A killed run must always be recoverable from
git + `current-phase.md` alone, with nothing living only in agent memory.

## 2. Merge policy — **resolved: self-merge on green**

The plan says phases "end with a merged PR" but never says who merges. **Decision:
self-merge on green.** The agent merges its own PR once *all required checks pass*
(the full gate set: build, tests, sanitizers, nflint, tbcover, abi-guard,
trace-schema-guard, depcheck/toolcheck, boundary-check, golden-frame-diff,
app-smoke where applicable, pr-checklist bot). CI and the STOP gates are the
quality bar; the human is reserved for blocker adjudication only — exactly what
"available for blocker-doc adjudication, not 'is this approach OK?'" implies.

For this to be *structurally* safe rather than honesty-based, the merge gate is
mechanical: with `ci-mode=actions`, branch-protection required checks block a red
merge even if the agent attempts one (verified by preflight §5). The rejected
alternative — a human merge gate — would put human-review latency on every
subphase and is not walk-away autonomy.

"Required checks" = the protected-branch required-status-checks list configured
in preflight. A check that is merely present but not *required* does not gate the
merge. If a required check is missing from branch protection, that is a preflight
failure, not something the agent works around.

## 3. Timebox semantics (active work, not calendar days)

The plan's timeboxes are written in elapsed days. An agent invoked sporadically
on an evenings-and-weekends cadence will blow every day-based box during
wall-clock time it spent idle. Timeboxes are therefore reinterpreted in units the
agent actually controls:

- **Primary unit: iteration cycles on the same target.** A "cycle" is one
  build/test/diagnose/fix attempt against a single failing subphase target. Each
  plan timebox maps to a cycle budget recorded in `current-phase.md`
  (`Timebox: N cycles`). Default mapping: 1 plan-day = 6 cycles, rounded.
- **Secondary unit: cumulative active hours** on the subphase, logged in
  `current-phase.md`, as a backstop against thrash that doesn't trip the cycle
  count.
- **Stop-condition (was "2× timebox").** Halt when *either* the cycle budget is
  exceeded by 2× *or* active hours exceed 2× the mapped estimate. Either trip
  produces a blocker doc — the plan didn't anticipate something.

Calendar dates still appear in `current-phase.md` for the human's benefit, but
they never trigger a stop.

## 4. Budget ceiling, heartbeat, and kill switch — enforced by the supervisor

A walk-away loop must be bounded and observable. The agent's own stop rules are
honest-effort; the failure mode this guards against is a *flailing* agent, which
is the least likely to honor them. So these three live in the external supervisor
(`scripts/supervise.sh`), where the agent cannot defeat them. The agent's job is
to keep durable in-repo state truthful; the supervisor does the enforcing and the
relaying.

**Budget ceiling (per supervisor session).** Hard caps held by the supervisor
(`NF_MAX_HOURS`, `NF_MAX_ITERS`, `NF_MAX_STUCK`, `NF_ITER_TIMEOUT_SEC`) and echoed
into `current-phase.md` at launch: wall-clock hours, max Codex relaunches, max
consecutive no-progress iterations (catches global flailing the per-subphase cycle
budget would miss), and a per-iteration watchdog. Hitting a cap is a
**budget-exhaustion halt**, not a blocker — the supervisor relays a
`BUDGET-EXHAUSTED` heartbeat and stops; nothing is wrong with the *plan*, the run
simply spent its envelope and waits for the human to extend it.

These are *per-session* caps. Phase A is ~50 working days of agent work, so it
spans many supervisor sessions (evenings/weekends); each `supervise.sh` launch is
bounded by these ceilings, and you relaunch to continue. Size `NF_MAX_HOURS` /
`NF_MAX_ITERS` to a session you're comfortable leaving unattended, not to the
whole project — the 0–3 spike defaults are likely too small now.

**Kill switch.** `docs/HALT`. The supervisor checks it before and after every
iteration and stops cleanly if present; the agent also checks it on reconciliation
(§1) as a courtesy and finishes its current commit before yielding. The human can
drop the file at any time to stop the loop without force.

**Heartbeat (mobile-first, two senders).** The primary channel is the
maintainer's phone, via the send-only helper `scripts/notify.sh` (pluggable
backend: Slack / ntfy / Pushover / generic webhook; severity maps to provider
priority so blockers buzz and routine progress stays quiet). Two senders use it:

- The **supervisor** relays state transitions between iterations and the things
  only it can see — loop quiet, died, budget exhausted, stuck, halted — at the
  appropriate severity.
- The **agent** may push in-the-moment alerts (it calls `notify.sh`; it never
  reads the secret, which the helper takes from the environment): `urgent` when it
  opens a blocker and halts, `high` at a phase milestone or run completion.
  Routine per-cycle progress is the supervisor's job, not the agent's.

Durable in-repo state (`current-phase.md`, blocker docs) remains the source of
truth; the notify is a courtesy buzz layered on top, never a substitute for the
doc. Letting the agent send is a bounded risk — a notification hook is send-only
with tiny blast radius, unlike the merge token, which the agent still never
holds.

## 5. Sequencing and the no-parallel-work rule

The plan is strictly sequential and forbids reordering. This loop honors that
literally: an open blocker on phase N halts the *whole* loop. The agent does not
skip ahead to an independent subphase to "stay productive" — that would reorder
the plan and erode the audit trail the gates exist to produce.

The consequence is that blocker *frequency* is the dominant cost of unattended
operation: each blocker can cost days of wall-clock idle while the human
surfaces. This is the reason the preflight gate (`docs/preflight.md`) is
load-bearing — most day-one blockers are missing provisioning, not genuine plan
gaps, and every one of those resolved up front is idle time avoided.

## 6. Blocker notification and resume protocol

Writing the blocker doc is necessary but not sufficient for a walk-away loop — an
unread doc in a directory is a silent stall.

**On STOP:** produce `docs/blockers/<phase>-<short>.md` per the template, commit
and push it (with the agent's recommended option in the doc), send one `urgent`
mobile alert via `scripts/notify.sh`, then halt. The pushed doc is the contract;
the notify is the courtesy buzz. If the agent halts mid-iteration without sending,
the supervisor still detects the unresolved blocker between iterations and relays
it — so the maintainer is alerted either way.

**Human adjudication signal (unambiguous):** the human appends an
`## Adjudication` block to the blocker doc:

```markdown
## Adjudication
**Decision:** <chosen option, or new direction>
**Rationale:** <one or two lines>
**Decisions-log entry:** <link, if this sets a precedent>
**Resume:** yes
```

`Resume: yes` is the only signal that unblocks the loop. A blocker doc without an
`## Adjudication … Resume: yes` block keeps the loop halted, no matter what is
said in Slack or elsewhere — there must be exactly one durable, in-repo source of
the unblock.

**On resume:** the agent detects the adjudication on its next reconciliation
(§1), applies it, links the adjudication from any resulting `decisions-log.md`
entry, moves the blocker doc to a resolved state (rename to
`docs/blockers/resolved/<phase>-<short>.md`), updates `current-phase.md`, and
continues.

## 7. Run scope — **Phase A (build the app)**

The initial run was a spike bounded to the Phases 0–3 checkpoint, to test the
thesis "an agent can be trusted to drive this gated pattern unattended." That
checkpoint was reached and the thesis held (decisions-log). **Scope is now lifted
to all of Phase A:** the agent continues sequentially from the current phase
through Phase 10 (ship Phase A). Phases 0–3 deliverables exist and are not
rebuilt.

This brings external dependencies that the spike deliberately avoided into play —
local Developer ID signing (Phase A), maintainer-manual real-module validation
(Phase 8), and the non-deterministic Metal golden layer (Phase 6) — see §4 budget
notes and `preflight.md`. On reaching Phase 10 the agent records the terminal
marker the supervisor watches for — exactly `**Run status:** complete` in
`current-phase.md` — and treats it as a clean halt (no blocker doc; supervisor
relays success). Phase B (signing/notarization/distribution, the name gate) and
Phase C are out of scope without explicit direction.

## 8. "Run until" — consolidated halt conditions

The agent runs continuously between halts. It halts on exactly one of (heartbeats
are relayed by the supervisor, not emitted by the agent):

1. **Run boundary reached** (§7) — Phase A complete; clean halt; agent sets
   `**Run status:** complete`, supervisor relays success.
2. **STOP gate triggered** — agent writes + pushes the blocker doc and halts;
   supervisor relays a blocker heartbeat and waits for adjudication (§6).
3. **Subphase exceeds 2× its cycle/active-hour budget** (§3) — blocker doc.
4. **Plan contradiction or uncovered situation** — blocker doc requesting
   guidance. Do not invent.
5. **CI unrecoverably broken** — two consecutive merged-or-attempted PRs failing
   the same gate the agent cannot diagnose from trace output — blocker doc.
6. **Budget exhausted** (§4) — supervisor stops, relays `BUDGET-EXHAUSTED`.
7. **Kill switch present** (§4) — `docs/HALT`; supervisor stops, relays `HALTED`.
8. **Stuck** — supervisor sees `NF_MAX_STUCK` consecutive iterations with no
   forward progress *and* no blocker doc; it stops for inspection. This catches
   flailing-in-place that no agent self-check would surface.

It does not halt because work is hard or slow. Between halts, it keeps going.
