# Current Phase

Phase 3 first-agent checkpoint is complete. The Phases 0-3 spike boundary has
been reached, and Phase 4 has not been started.

## Status

- Imported bootstrap docs and scripts on 2026-05-28.
- Reconstructed `nightfall-plan-macos.md` from the imported bootstrap docs.
- Reconstructed `nightfall-agent-prompt-macos-v2.md` from the imported bootstrap docs.
- Added bootstrap clean-room and blocker workflow scaffolding.
- Added `third_party/VERSIONS.toml` with the accepted default pinning policy.
- Added minimal `nightfall_core` C ABI skeleton.
- Added local CMake/CTest build path.
- Added initial `boundary-check` and `nflint` gates with meta-test fixtures.
- Completed manual Phase 0 bring-up loop: `scripts/ci/run-local.sh` is green.

## Completed Work

- Continued Phase 0 gate foundation in manual mode.
- Completed `depcheck` and `toolcheck` with gate meta-tests.
- Completed `trace-schema-guard` with gate meta-tests.
- Added `tbcover` with gate meta-tests.
- Added 68k assembler fixture smoke check with gate meta-tests.
- `scripts/ci/run-local.sh` is green with build, CTest, six gates, assembler
  fixture smoke, and gate meta-tests.
- Began Phase 1 with a cited A-line trap dispatch scaffold. This does not
  implement Toolbox routine behavior yet.
- Added `docs/phase-1-checklist.md` to make Phase 1 completion auditable.
- Added a cited bootstrap fixture runner that dispatches A-line trap words and
  stops on `RTS` without implementing general 68k semantics.
- Converted C++ tests to the locked doctest framework and recorded the vendored
  header pin.
- Reworked core context creation to use a deterministic fixed static pool
  instead of heap allocation.

## Phase 1 Completion

- Deterministic core context exists behind the C ABI.
- Public source citations are recorded for A-line trap dispatch and the fixture
  runner before implemented behavior.
- A-line trap classification and host-handler dispatch scaffold are implemented.
- Bootstrap fixture runner dispatches A-line trap words and stops on `RTS`.
- Core units have co-located tests, fixtures, and documentation enforced by
  `tbcover`.
- `scripts/ci/run-local.sh` passes.

## Phase 2 Progress

- Started ABI hardening with a Mach-O-friendly snapshot guard for exported
  `nf_*` symbols and the public C header.
- Added required ASan/UBSan sanitizer profile and best-effort TSan profile to
  the local verification runner.
- Added `docs/phase-2-checklist.md` and `docs/sanitizers.md`.
- UBSan passes locally through the dedicated sanitizer smoke harness; TSan is
  attempted and skipped when unsupported by the host toolchain.
- ASan is temporarily best-effort on this macOS 26.5 host by maintainer
  adjudication. Allocator-using ASan binaries deadlock during sanitizer runtime
  initialization before Nightfall code; minimal ASan probes and Mozilla Bug
  2037587 corroborate this as a runtime/toolchain issue rather than Nightfall
  code.
- Added GitHub Actions `local-gates` workflow and configured branch protection to
  require it, restoring the mechanical self-merge-on-green gate.

## Phase 2 Completion

- Mach-O-friendly ABI snapshot guard protects the public C header and exported
  `nf_*` symbols.
- ABI guard has known-good and known-bad meta-tests.
- Trace, boundary, coverage, dependency, tool, lint, assembler, ABI, and
  sanitizer meta-gates are wired into local verification.
- UBSan passes as the required sanitizer profile; TSan is attempted and skipped
  with a host/toolchain reason when unsupported.
- ASan is temporarily best-effort on this macOS 26.5 host by the 2026-06-03
  maintainer adjudication in `docs/decisions-log.md`.
- `scripts/preflight.sh` passes with warnings only.
- `scripts/ci/run-local.sh` passes with the adjudicated ASan warning and existing
  non-blocking dependency-pin warning.

## Phase 3 Completion

- Phase 2 merged via PR #1 with the required `local-gates` GitHub check passing.
- Active blocker directory is clear; resolved blockers preserve the ASan and
  branch-protection adjudication trail.
- Preflight passes on the macOS build host with the Slack notification test
  enabled via the local launch environment.
- Local gate suite passes on the macOS build host.
- `docs/phase-3-checklist.md` records the checkpoint evidence.
- No Phase 4 work has begun.

## Spike status

**Spike status:** complete
