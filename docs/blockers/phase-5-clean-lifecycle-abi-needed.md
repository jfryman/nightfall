# Blocker: phase-5-clean-lifecycle-abi-needed

## Summary

Phase 5 cannot complete the Flying Toasters 10-minute end-to-end checkpoint
because the repository still lacks a clean sanctioned source or sanitized
maintainer artifact for the After Dark graphics-module lifecycle ABI.

## Context

- Phase/subphase: Phase 5 checkpoint.
- Gate or invariant: Phase 5 requires a real-module 10-minute end-to-end run with
  zero unimplemented-trap warnings; clean-room discipline forbids deriving the
  affected lifecycle ABI from the Berkeley Systems sample/header material exposed
  in the prior session.
- First observed: 2026-06-06 after fresh-session reconciliation of
  `docs/blockers/resolved/phase-5-after-dark-lifecycle-contamination.md`.
- Command or check:
  - `rg -n "module-loading|ADgm|selector|kInit|kBlank|kDraw|lifecycle" docs core tools nightfall-plan-macos.md`
  - `build/tbtrace .nightfall/manual-modules/Flying\ Toasters --cycles 60000000`
  - `scripts/ci/run-local.sh`

## Evidence

- The fresh session completed the non-lifecycle backfill authorized by
  `docs/blockers/resolved/phase-5-missing-real-module-runner.md`:
  `nf_module_load`, `nf_module_start`, `nf_advance`,
  `nf_module_framebuffer`, `nf_module_stop`, and
  `nf_context_set_random_seed` are now present in the public C ABI and covered by
  a synthetic resource-fork xattr test.
- Local gates pass after that backfill: `scripts/ci/run-local.sh`.
- The ignored maintainer-supplied module can still be parsed and entered only at
  the raw `ADgm/0` entry:
  - resource fork bytes: 30903
  - resource count: 27
  - `ADgm` count: 1
  - `ADgm/0` bytes: 12402
  - result: `status: ok`, `stop-reason: rts`, `cycles-run: 116`,
    `trap-count: 0`, `unimplemented-trap-count: 0`
- That raw entry returning immediately is not the Phase 5 end-to-end checkpoint.
  It does not establish the `kInit` / `kBlank` / draw lifecycle, parameter block
  layout, selector values, register/stack convention, or 10-minute module run.
- No committed `docs/module-loading-abi.md` or equivalent sanitized lifecycle ABI
  artifact exists in the repository.

## Requested Adjudication

Please choose the smallest clean-room resume path:

1. Provide a maintainer-authored sanitized ABI artifact covering only the needed
   graphics-module call convention, selector/message values, parameter block
   layout, and entry sequence, with provenance acceptable to the project.
2. Explicitly approve a black-box observation protocol for deriving the lifecycle
   ABI from executing the maintainer-supplied local module, including what
   observations may be recorded in committed sanitized artifacts.
3. Adjudicate that Phase 5 may close on the Toolbox manager implementation,
   resource-fork runner, module C ABI backfill, and raw `ADgm/0` zero-trap probe,
   while carrying the first true lifecycle run to a later phase.

Recommended option: 1. It lets Phase 5 complete the intended checkpoint without
relying on contaminated source exposure or weakening the end-to-end claim.

## Resume

Resume only after the maintainer appends an adjudication block below with
`**Resume:** yes`.

## Adjudication 2026-06-06

Decision:

**Resume:** no
