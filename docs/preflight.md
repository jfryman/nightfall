# Nightfall — Preflight Gate

The line you cross to walk away. **Nothing in the loop is autonomous until every
item here is true.** Most day-one agent stalls are missing provisioning, not
genuine plan gaps (see autonomous-operation.md §5), and an unattended loop turns
each one into days of idle wall-clock time. Resolve them once, here, up front.

The agent's first action is to verify the machine-checkable items below and refuse
to start — with a blocker doc — if any fail. The human-judgment items it cannot
check; those are on you.

**This checklist is executable.** `scripts/preflight.sh` is the machine-checkable
half, made runnable: it probes every permission, credential, toolchain, and
network door, prints `PASS / WARN / FAIL / SKIP` with a remediation line each, and
exits non-zero if any hard requirement is unmet. It authorizes nothing itself —
you do the granting; it tells you what is still locked. The supervisor
(`scripts/supervise.sh`) runs it before launching Codex and will not launch on a
non-zero exit; the agent re-runs it on every reconciliation as a cheap sanity
check. Workflow:

```
./scripts/preflight.sh            # see what's red
<authorize the FAIL items>        # grant egress / scopes / install tools
./scripts/preflight.sh            # repeat until exit 0
```

The human-judgment items below (the build host, the merge-policy intent, the
clean-room attestation language) are not script-checkable and are recorded in
`docs/decisions-log.md` instead.

---

## A. Repository and merge surface

- [ ] Git repository created, remote set, agent can push.
- [ ] Protected primary branch (`main`) configured with **required** status
      checks = the full gate set: build, tests, sanitizer matrix, `nflint`,
      `tbcover`, `abi-guard`, `trace-schema-guard`, `depcheck`, `toolcheck`,
      `boundary-check`, `golden-frame-diff`, `app-smoke` (where applicable),
      pr-checklist bot. A check that is present but not *required* does not gate
      a merge.
- [ ] Merge policy **decided: self-merge-on-green** (recorded in
      `decisions-log.md`). The agent's token has merge rights on the protected
      branch, and the required-checks list above makes a red merge mechanically
      impossible — so self-merge rests on the gate, not on agent honesty.

## B. Build / test / install targets

- [ ] **Build/install target decided: `smugglebook`** (Apple Silicon, macOS 26,
      arm64). The project is macOS-only; CI runners cannot install a `.saver` into
      a human's `~/Library/Screen Savers/`, and several validation steps are
      Mac-local — `smugglebook` is where the agent builds, smoke-tests, and (in
      Phase A, beyond this spike) installs.
- [ ] CI runner: **`smugglebook` registered as a self-hosted macOS runner**
      (so branch-protection required checks execute and gate merges). Verified by
      `preflight.sh` §5 under `ci-mode=actions`.
- [ ] Xcode + macOS versions on the runner recorded in `VERSIONS.toml`.
- [ ] Machine kept awake while the loop runs (the supervisor under `caffeinate`;
      `pmset -c sleep 0`) with a logged-in session for the GUI `app-smoke`. A
      sleeping laptop is a silently stalled loop.

## C. Secrets (provisioned, scoped, never committed)

- [ ] GitHub token scoped to repo + actions, available to the agent via
      environment/CI secret — never written to the tree. `nflint` already bans
      committed secrets; confirm it covers token shapes.
- [ ] Phase A signing: Developer ID cert present in the build host's keychain
      for local signing. (Q19 KMS/OIDC/Terraform is Phase B — not required now;
      record that explicitly so the agent doesn't attempt it.)
- [ ] No other credentials required for Phases 0–3. (Telemetry is none per Q21;
      the only outbound is the Q18 weekly Releases check, read-only.)

## D. Pinned versions and toolchain decisions

`VERSIONS.toml` is referenced as the source of truth across the plan but ships
empty. Pin these before launch, or the agent blocks on its first action (it is
forbidden to invent them):

- [ ] **Musashi** — exact commit.
- [ ] **nlohmann::json** — exact tag.
- [ ] **glsl-crt (wessles)** — exact commit.
- [ ] **vasm** — exact version (fixture assembler).
- [ ] **abidiff / libabigail** — exact version.
- [ ] **C++ test framework** — recommend doctest (header-only, fast, headless).
- [ ] **Fuzz engine** — recommend libFuzzer (system clang on Apple Silicon).
- [ ] **Sanitizer set** — ASan + UBSan required; **TSan flagged** (limited on
      Apple Silicon — decide whether it's required-pass or best-effort).
- [ ] **Xcode project generation** — recommend XcodeGen with a committed
      `project.yml`, or CMake's Xcode generator as the single build authority.
      Do not hand-author `pbxproj`.

Default pin policy if you'd rather not hand-pick: "latest stable tag/release as
of bootstrap date, hash recorded in `VERSIONS.toml`, `DEP-UPDATE:` tag required
to change." Acceptable for the spike; revisit before Phase B.

## E. Observability of the loop itself

- [ ] Mobile notifications configured — the primary channel. Pick a backend via
      `NF_NOTIFY_BACKEND` (`slack` | `ntfy` | `pushover` | `webhook`) and set its
      secret: Slack can use `NIGHTFALL_SLACK_WEBHOOK` or a macOS Keychain generic
      password with service `nightfall/slack-webhook` and account `$USER`;
      alternatives use `NTFY_URL` (+`NTFY_TOKEN`), `PUSHOVER_TOKEN`+
      `PUSHOVER_USER`, or `NF_WEBHOOK_URL`. Both the supervisor and the agent
      push through `scripts/notify.sh`; severity maps to provider priority so
      blockers buzz and routine progress stays quiet. The agent calls the helper
      and **never holds the raw secret** (the helper reads it from the environment
      or Keychain). Verify with
      `./scripts/preflight.sh --test-notify` (sends one alert to your phone).
- [ ] Budget ceiling set as supervisor env vars at launch: `NF_MAX_HOURS`,
      `NF_MAX_ITERS`, `NF_MAX_STUCK`, `NF_ITER_TIMEOUT_SEC`.
- [ ] Kill switch wired: `docs/HALT` (checked by the supervisor before/after every
      iteration and by the agent on reconciliation).
- [ ] `.nightfall/` (supervisor logs/state) added to `.gitignore`.
- [ ] `NF_CODEX_CMD` set to your Codex version's non-interactive/full-auto
      invocation with network + git allowed (the default
      `codex exec --full-auto` is a starting point — **verify against your
      version**; `--dry-run` won't catch a wrong invocation).

## F. Gates that test the gates

The agent *builds* the enforcement (nflint, tbcover, abi-guard, boundary-check,
the PR bot) during Phase 0, then is supposedly governed by it. Nothing closes
that loop unless the gates have their own tests. Make this a Phase 0 exit
criterion, not just the four-PR demo:

- [ ] Each gate ships committed **known-bad** and **known-good** fixtures, run in
      CI, asserting the gate fails *for the right reason* and passes otherwise
      (a file that must trip the nflint forbidden-source scan; an untagged ABI
      break that must trip abi-guard; a framebuffer-only test that must trip
      tbcover; etc.). This generalizes the plan's existing "deliberate-crash
      fixtures" idea to every gate.
- [ ] Golden-frame strategy split: **exact hash** for the deterministic core
      software framebuffer; **tolerance/perceptual diff** for offscreen *Metal*
      output (Phase 6), which is not bit-stable across GPU/driver/OS-image
      updates. Recorded in `decisions-log.md` so the agent doesn't chase driver
      noise as regressions.

## G. Clean-room for an AI contributor (review + sign before first commit)

The human-worded attestation ("has not read forbidden sources: Basilisk II,
SheepShaver, Wine, qemu, …") cannot be made truthfully by an LLM agent — those
corpora are very likely in its training data, and it can reproduce contaminating
code from memory without "reading" anything in-session. The legal premise needs
AI-specific adaptation before any code is written:

- [ ] **Attestation reworded for an AI contributor** (drafted in the bootstrap
      prompt; **review with counsel before relying on it for Phase B**): the
      agent affirms its output derives *solely from public specs cited in-session*
      in `clean-room-sources.md`, that it will flag any output it recognizes as
      originating from a forbidden source, and that the human maintainer holds the
      legal attestation of record.
- [ ] **`nflint` denylist promoted to primary defense** — the only mechanism that
      works against parametric recall. Treat the denylist (distinctive
      identifiers, structural signatures) as a first-class, tested artifact, not
      a footnote.
- [ ] **Citation-before-implementation enforced** — every trap's `// Source:`
      header must cite a specific Inside Macintosh page/section actually
      consulted; `tbcover` already requires the `.md` citation file, so extend it
      to reject empty/placeholder citations.

## H. Phase 0.5 is non-blocking

Trademark search and legal-review scheduling are human/legal judgments an
autonomous agent should not make and may not be able to (network/legal). Phase
0.5 currently sits in the sequential path before Phase 1.

- [ ] Confirm Phase 0.5 is **non-blocking**: "Nightfall" stays a placeholder, the
      agent proceeds to Phase 1 on it, and the *only* hard name gate is Phase B.
      Recorded in `decisions-log.md`.

---

## Decision queue — resolved (recorded in `docs/decisions-log.md`)

Settled:

1. **Merge policy** — ✅ self-merge-on-green (mechanical via branch-protection
   required checks under `ci-mode=actions`).
2. **macOS build/install target** — ✅ `smugglebook` (Apple Silicon, macOS 26,
   arm64). Also the self-hosted CI runner.
3. **Spike scope** — ✅ Phases 0–3 checkpoint, terminal.
4. `VERSIONS.toml` pins — ✅ default policy (latest stable as of bootstrap, hash
   recorded, `DEP-UPDATE:` to change).
5. Xcode project generator — ✅ XcodeGen (`project.yml`); CMake stays the
   authority for `libnightfall`.
6. Test / fuzz / sanitizers — ✅ doctest; libFuzzer; ASan + UBSan required, TSan
   best-effort (not a required check this spike).
7. Notification substrate — ✅ Slack webhook, **relayed by the supervisor** (the
   agent writes durable in-repo state and never holds the secret).

Still open:

8. **AI-adapted clean-room attestation language** — drafted in the bootstrap
   prompt and §G above; needs your sign-off and a counsel review before it is
   relied on for Phase B. Not a blocker for the 0–3 spike (personal-use only), but
   the one item left to close.

Two forks remain knobs, not blockers: `ci-mode` (`actions` recommended and
assumed by the merge-gate decision; `local` still selectable) and the supervisor
budget ceilings (`NF_MAX_HOURS` / `NF_MAX_ITERS` / `NF_MAX_STUCK` /
`NF_ITER_TIMEOUT_SEC`), which you tune at launch.
