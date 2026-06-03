# Nightfall — Decisions Log

Append-only. Each entry: date, context, decision, rationale, consequences.
Decisions are recorded as made, never accumulated. The agent adds entries during
execution; the entries below capture the pre-execution decisions the rescope and
the autonomy planning settled, and are referenced throughout the plan, the
bootstrap prompt, `autonomous-operation.md`, and `preflight.md`.

The 22 committed design decisions (Q1–Q22) live in `nightfall-plan-macos.md`;
this log holds everything decided since, plus anything decided during execution.

---

### 2026-05-28 — macOS-only rescope
**Context:** The cross-platform (macOS + Linux + Windows) scope was the largest
source of maintenance burden and agent-execution risk and added nothing to the
actual goal. **Decision:** macOS only — native Metal (no Vulkan/MoltenVK), no
cross-platform CI, Windows modules cut, PowerPC kept as a deprioritized Phase C
someday-maybe. **Rationale:** removes Vulkan/MoltenVK/SPIR-V, equivalence testing,
the portability tax across every phase, and three-runner CI. **Consequences:**
~48–52 working days for Phase A vs ~82; the C ABI boundary is retained for
*testability*, not portability.

### 2026-05-28 — Merge policy: self-merge on green
**Context:** The plan said phases end with a merged PR but never said who merges;
this is the central autonomy question. **Decision:** the agent self-merges its own
PR once all *required* status checks pass. **Rationale:** matches "maintainer
available for blocker adjudication, not 'is this approach OK?'"; a human merge gate
would put review latency on every subphase and is not walk-away autonomy.
**Consequences:** requires `ci-mode=actions` with branch-protection required checks
so a red merge is mechanically impossible — the gate, not agent honesty, enforces
quality.

### 2026-05-28 — Build/CI target: smugglebook
**Context:** macOS-only needs a Mac; the maintainer's primary workstation is Arch.
**Decision:** `smugglebook` (Apple Silicon, macOS 26, arm64) is the build host and
the self-hosted macOS CI runner. **Rationale:** it's the available Mac and
co-locating build + CI keeps the loop fast and cheap. **Consequences:** the loop
only advances while the machine is awake and logged in (GUI `app-smoke`); the
supervisor runs under `caffeinate`, sleep-on-AC disabled. CI is local/self-hosted,
not cloud.

### 2026-05-28 — Spike scope: Phases 0–3 checkpoint
**Context:** "Spike" (bounded uncertainty-reduction) vs the prompt's original
"run to Phase A complete." **Decision:** this run terminates at the first-agent
checkpoint (Phases 0–3). **Rationale:** that slice exercises every gate, the trace
pipeline, the clean-room paper trail, and the blocker/resume loop — the actual
thesis — with the fewest external dependencies (no real `.AD` in the core path, no
signing, no licensed-content surface). **Consequences:** the agent stops at the
checkpoint (sets `**Spike status:** complete`) regardless of remaining budget and
does not begin Phase 4.

### 2026-05-28 — Dependency pinning: default policy
**Context:** `VERSIONS.toml` is the cited source of truth but shipped empty; the
agent is forbidden to invent pins. **Decision:** default policy — latest stable
tag/release of each dep as of bootstrap, exact hash recorded in `VERSIONS.toml`,
`DEP-UPDATE:` commit tag required to change. **Rationale:** removes the day-one
stall without the maintainer hand-picking five commits. **Consequences:** pins are
recorded, not chosen ad hoc; revisit before Phase B.

### 2026-05-28 — Build tooling: CMake + XcodeGen
**Context:** Hand-authoring `pbxproj` is a known agent thrash point.
**Decision:** CMake is the authority for `libnightfall` (C++); XcodeGen generates
`Nightfall.xcodeproj` from a committed spec; they meet at the C ABI seam.
**Rationale:** each tool does what it's best at; the project layout already drew
this boundary. **Consequences:** XcodeGen must be present on the build host (it
is); no hand-edited project files.

### 2026-05-28 — Test / fuzz / sanitizers
**Context:** Frameworks were unspecified; the agent would otherwise invent them.
**Decision:** doctest (C++ unit tests); libFuzzer (`-fsanitize=fuzzer`); ASan +
UBSan required-pass; TSan best-effort, not a required check this spike.
**Rationale:** doctest is header-only and fast (kind to the loop); libFuzzer ships
with the toolchain; the core is single-threaded-per-context through Phase 3, so
TSan has little to bite on until the renderer/audio threading of Phase 6+.
**Consequences:** recorded so the agent doesn't choose; revisit TSan when real
threading lands.

### 2026-05-28 — Golden-frame strategy split
**Context:** The plan treated `golden-frame-diff` as one exact-hash mechanism, but
offscreen Metal output is not bit-stable across GPU/driver/OS-image updates.
**Decision:** exact hash for the deterministic core software framebuffer;
tolerance / perceptual diff for rendered Metal output. **Rationale:** prevents
chasing driver noise as regressions in Phase 6. **Consequences:** out of this
spike's scope (Metal lands Phase 6), but the agent must not exact-hash Metal output
if it gets there.

### 2026-05-28 — Gate-meta-tests are a Phase 0 exit criterion
**Context:** The agent *builds* the enforcement (nflint, tbcover, abi-guard,
boundary-check) then is governed by it; nothing closes that loop unless the gates
are themselves tested. **Decision:** every gate ships committed known-bad and
known-good fixtures, run in CI, asserting it fails for the right reason and passes
otherwise; Phase 0 is not done at the four-PR demo alone. **Rationale:** a gate
never shown to catch its target class is not load-bearing. **Consequences:**
generalizes the plan's "deliberate-crash fixtures" idea to every gate.

### 2026-05-28 — Phase 0.5 is non-blocking
**Context:** Trademark search / legal scheduling sat in the sequential path before
Phase 1; an autonomous agent should not (and likely cannot) make that call.
**Decision:** Phase 0.5 is non-blocking — "Nightfall" stays a placeholder, the
agent proceeds to Phase 1, the only hard name gate is Phase B. **Rationale:**
keeps a legal/judgment task off the critical path. **Consequences:** the agent
does not attempt trademark clearance.

### 2026-05-28 — Clean room adapted for an AI contributor
**Context:** The human attestation ("has not read forbidden sources: Basilisk II,
SheepShaver, Wine, qemu, …") cannot be made truthfully by an LLM, whose training
data likely includes them, and which can reproduce contaminating code from memory.
**Decision:** AI-adapted attestation (the agent affirms output derives solely from
public specs cited in-session and flags recognized forbidden-source provenance);
`nflint`'s denylist is the *primary* defense; citation-before-implementation is
enforced (no trap without its `// Source:` to a specific Inside Macintosh page).
The maintainer holds the legal attestation of record. **Rationale:** the only
mechanisms that work against parametric recall are the denylist and mandatory
in-session citation. **Consequences:** attestation template at
`docs/attestations/TEMPLATE.md`; **needs counsel review before Phase B relies on
it.** Not a blocker for the 0–3 spike (personal use).

### 2026-05-28 — Execution model: Codex local, external supervisor
**Context:** The two-tier Claude-architect / Pip-worker model; the maintainer
chose to run this spike via Codex locally on `smugglebook`. **Decision:** the
frozen plan + docs are the architect output, Codex is the sole worker, the
maintainer adjudicates blockers; a thin external supervisor (`scripts/supervise.sh`)
runs preflight, enforces the budget ceiling / kill switch / crash-restart from
outside the agent, and classifies each Codex exit by repo state. **Rationale:** a
flailing agent is the least likely to honor its own stop rules, so the hard
ceilings must live outside it. **Consequences:** `AGENTS.md` is the in-context
contract; `docs/HALT` is the kill switch; budget knobs are supervisor env vars.

### 2026-05-28 — Notifications: mobile-first, two senders, send-only helper
**Context:** The maintainer wants Codex able to reach their phone directly, not
only via the supervisor. **Decision:** a shared send-only helper
`scripts/notify.sh` (pluggable backend: Slack / ntfy / Pushover / generic webhook;
severity → provider priority) is called by *both* the supervisor (liveness,
death, budget, stuck, transitions) and the agent (in-the-moment: `urgent` on a
blocker, `high` on a milestone). **Rationale:** a notification hook is send-only
with tiny blast radius, unlike the merge token, so letting the agent send is a
bounded risk; the helper reads the secret from the environment so it never enters
the agent's prompt. **Consequences:** durable in-repo state remains the source of
truth; the notify is a courtesy buzz. Revises the earlier "agent never touches
Slack" stance.

### 2026-06-03 — ASan macOS 26.5 host exception
**Context:** Phase 2 requires ASan and UBSan, but on this macOS 26.5 host every
tested ASan runtime that performs meaningful allocator/global/stack
instrumentation deadlocks during sanitizer initialization before Nightfall code
executes. UBSan passes, and the failure matches a public upstream compiler-rt
Darwin ASan issue. **Decision:** ASan is temporarily best-effort only on this
macOS 26.5 host; UBSan remains required-pass, and ASan becomes required-pass
again once the host/toolchain contains the upstream compiler-rt fix. **Rationale:**
this preserves local Phase 2 momentum without weakening the project-wide intent
to require ASan on a functioning Darwin sanitizer runtime. **Consequences:**
`scripts/ci/sanitizers.sh` may warn and continue for ASan timeouts on this host;
the exception is documented in `docs/sanitizers.md` and was adjudicated in
`docs/blockers/resolved/phase-2-asan-macos-26.md`.

---

### Open (not yet decided)
- **AI-adapted attestation language — counsel review.** Drafted; needs the
  maintainer's sign-off and an attorney look before Phase B. Tracked in
  `docs/preflight.md` §G.
- **abi-guard mechanism on Mach-O.** `abidiff`/libabigail is ELF/DWARF-oriented;
  the C ABI is Mach-O. Verify Mach-O support or substitute a symbol-dump/header-
  snapshot diff during Phase 2 — if unclear, blocker doc, do not invent. (Risk,
  not yet a decision.)
- **ci-mode.** `actions` assumed by the merge-policy decision; `local` remains
  selectable as a knob.
