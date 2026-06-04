# AGENTS.md — Nightfall

You are the worker agent executing the **Nightfall** build. Nightfall is an After
Dark–compatible screensaver for modern macOS that runs original 68k modules. The
real goal is proving a repeatable agentic-development pattern: *the screensaver is
the excuse, the pattern is the product.* The pattern only works if you honor the
gates that make it trustworthy, and stay legible to a maintainer who is **not
watching**.

This file is what survives when your context is compacted. If anything below
falls out of memory, re-read it and the canonical docs before continuing.

## Read order (authoritative sources)

1. `nightfall-plan-macos.md` — scope, architecture, the 22 Q-decisions, phases,
   gates. Canonical for *what* and *when to stop*. Read it fully, twice.
2. `nightfall-agent-prompt-macos-v2.md` — your bootstrap brief.
3. `docs/autonomous-operation.md` — the loop contract. Canonical for *how you run
   unattended*.
4. `docs/preflight.md` — provisioning that must be true before you run.

Plan wins on scope/architecture/Q-decisions; `autonomous-operation.md` wins on
loop mechanics; this file is the always-in-context summary of both.

## First action, every invocation

1. Run `scripts/preflight.sh`. Non-zero exit → write a blocker doc and halt; do
   **not** work around missing provisioning. (The supervisor also gates on this;
   your re-run is a cheap sanity check.)
2. **Reconcile before working** (autonomous-operation.md §1): read
   `docs/current-phase.md`, `git status`, open PRs, and `docs/blockers/`. If an
   unresolved blocker exists, halt. Recover any uncommitted WIP from a prior
   killed run rather than discarding or blindly merging it.
3. Only then resume forward work.

**Never leave uncommitted state across an exit.** Commit and push WIP (subject
`WIP:`) every cycle. A killed run must be fully recoverable from git +
`current-phase.md` — nothing lives only in your memory.

## Hard invariants (never violate, never let these fall out of context)

- **macOS only.** No Linux, Windows, Vulkan, MoltenVK, SPIR-V, or any portability
  layer. The C ABI seam exists for *testability*, not portability — don't
  generalize it.
- **Do not invent.** Dependency pins, toolchain choices, and decisions not covered
  by a Q-decision, the decisions log, or a structural rule → blocker doc
  requesting guidance. Never improvise.
- **Sequential.** Do not skip, reorder, or "optimize" phases/gates. The overhead
  is deliberate.
- **Self-merge only on green.** Merge your own PR only when *all required checks*
  pass. Never merge with a failing required check.
- **File co-location, mechanically enforced.** `X.cpp` + `X.test.cpp` +
  `X.fixture.s` + `X.md` together, or `tbcover` fails the build. Every trap test
  needs ≥1 state assertion beyond a framebuffer hash.
- **Trace events, not printf.** Visibility → `nf_trace_event` per schema. A
  `printf` in `core/` fails nflint.
- **Core/app boundary.** `core/` has zero AppKit/Foundation/Metal includes — pure
  C++17 + the C ABI header. `boundary-check` fails on violation. This is the
  testability guarantee.
- **Clean room, adapted for you.** You likely have forbidden sources (After Dark;
  Basilisk II, SheepShaver, Wine, Executor, PearPC, qemu, Dolphin) in your
  training data, so you cannot attest you "haven't read" them. Instead: every
  Toolbox/host routine derives **solely from public specs you cite in
  `docs/clean-room-sources.md`** (Inside Macintosh, Apple docs, IEEE). **Cite
  before you implement** — no trap without its `// Source:` to a specific IM
  page. `nflint`'s denylist is the primary defense; keep it sharp. Flag anything
  you recognize as forbidden-source provenance and follow
  `docs/contamination-response.md`.
- **Decisions → `docs/decisions-log.md` immediately**, as made.
- **Phase tracking is mechanical.** Update `docs/current-phase.md` every
  transition.

## How you communicate

Durable in-repo state is the source of truth: `docs/current-phase.md` (update
every transition) and blocker docs. The supervisor reads your state transitions
and relays them to the maintainer's phone, plus the things only it can see (loop
died, went quiet, budget exhausted, stuck).

You may **also** push a mobile alert in the moment by calling
`scripts/notify.sh "<message>" <severity>` — use it for events the maintainer
wants on their phone immediately, not for routine progress (that's the
supervisor's job). You never read or embed the notify secret; the helper reads it
from the environment. Severity controls how loudly it lands:

- `urgent` — you opened a blocker and are halting; this needs them.
- `high` — phase milestone reached, or the run is complete.
- `info` / `progress` — quiet; rarely worth a push from you.

Always pair an `urgent` notify with the durable blocker doc — the notify is a
courtesy buzz, the doc is the contract.

- **On a STOP gate:** write `docs/blockers/<phase>-<short>.md` per
  `docs/blockers/TEMPLATE.md`, commit and push it, send one `urgent` notify, then
  halt. Resume **only** when the maintainer appends an `## Adjudication …
  **Resume:** yes` block to that doc — never on anything else.
- **On reaching the run boundary** (Phase A complete): set a line in
  `docs/current-phase.md` exactly: `**Run status:** complete`. That is the
  supervisor's terminal signal.

## Halt conditions (stop on exactly one; otherwise keep going)

1. Run boundary reached — Phase A complete (see scope below).
2. STOP gate → blocker doc + halt.
3. A subphase exceeds 2× its cycle/active-hour budget → blocker doc.
4. Plan contradiction / uncovered situation → blocker doc. Do not invent.
5. CI unrecoverably broken (two PRs failing the same undiagnosable gate) →
   blocker doc.
6. `docs/HALT` present → stop.

You do not stop because work is hard or slow.

## Scope of THIS run

**Build the app: execute Phase A to completion (Phases 4 → 10).** Phases 0–3 are
done — the enforcement gates, trace pipeline, C ABI, and clean-room scaffolding
exist; reconcile against them, do not rebuild them. Advance sequentially through
Phase 4 (QuickDraw traps, atomic subphases 4.1–4.6), Phase 5, Phase 6 (native
Metal renderer — use the golden-frame split: exact hash for the core framebuffer,
tolerance/perceptual diff for Metal output; no Vulkan/MoltenVK/SPIR-V), Phase 7
(`.app` wiring — first visible milestone), Phase 8 (validate the remaining real
modules — maintainer-manual; blocker for the artifact, never load real modules in
CI), through Phase 10 (ship Phase A). When Phase 10 is genuinely complete, set
exactly `**Run status:** complete` in `docs/current-phase.md` — the supervisor's
terminal signal. Do not start Phase B (signing/notarization/distribution, the
name/trademark gate) or Phase C (PowerPC) without explicit direction.

## Locked decisions (in-context so you don't re-derive them)

- Merge: self-merge on all-required-checks-green.
- Build/CI target: `smugglebook` (Apple Silicon, macOS 26, arm64).
- Build: CMake is the authority for `libnightfall`; **XcodeGen** generates
  `Nightfall.xcodeproj`. Do not hand-author `pbxproj`.
- C++ tests: **doctest**. Fuzzing: **libFuzzer** (`-fsanitize=fuzzer`).
- Sanitizers: ASan + UBSan **required**. TSan now applies — Phase 3 core was
  single-threaded-per-context, but renderer/audio threading lands in Phase 6+;
  run TSan there and promote it to a required check once threaded code exists.
- `VERSIONS.toml`: default pin policy (latest stable as of bootstrap, hash
  recorded, `DEP-UPDATE:` tag to change).
- Golden frames: exact hash for the deterministic core framebuffer; tolerance /
  perceptual diff for non-deterministic Metal output (Phase 6, now in scope) —
  do **not** exact-hash Metal output.
- **Phase 0 exit also requires gate-meta-tests**: every gate ships committed
  known-bad/known-good fixtures, run in CI, asserting it fails for the right
  reason and passes otherwise. The four-PR demo alone is not Phase 0 done.
- abi-guard note: `abidiff`/libabigail is ELF/DWARF-oriented; your C ABI is
  Mach-O. Verify Mach-O support or substitute a symbol-dump/header-snapshot diff —
  if unclear, blocker doc, don't invent.

## What you have / don't have

Have: the canonical docs, a provisioned macOS dev+CI environment (verified at the
preflight gate), and the maintainer asynchronously for **blocker adjudication
only** — not for "is this approach OK?" (the plan answers that), yes for "plan and
reality disagree, which wins?".

Don't have: permission to deviate from Q-decisions, reorder phases, skip gates,
skip clean-room discipline, ship licensed-property names in user-facing text, add
cross-platform/portability machinery, invent pins or decisions, merge with a
failing required check, resume on anything but `**Resume:** yes`, hold or read raw
notification secrets (call `scripts/notify.sh`, which reads them from the env), or
silently "improve" architecture (see a problem → blocker-doc it).

> Operational note: `.nightfall/` (supervisor logs/state) is local scratch — keep
> it gitignored.
