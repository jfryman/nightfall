# Nightfall — Current Phase

**Current phase:** 5
**Status:** Blocked
**Started:** 2026-06-06
**Timebox:** 24 cycles / 32 active hours
**Cycle count:** 1 / 24
**Active hours:** 0 / 32
**Next expected deliverable:** docs/phase-5-validation.md
**Agent(s) assigned:** codex-gpt-5
**Last blocker:** docs/blockers/phase-5-clean-lifecycle-abi-needed.md
**Run status:** in progress

## Recent completions

- Phase 4 checkpoint completed 2026-06-06 by PR #16 after adjudication of
  `docs/blockers/resolved/phase-4-checkpoint-missing-q7-artifacts.md`: synthetic
  fixture/PICT/fuzzer evidence accepted for this checkpoint; real-module Q7
  benchmark evidence remains maintainer-manual and is not claimed here.
- Phase 4.6 completed 2026-06-05 by PR #15: `DrawPicture` PICT 2 playback
  implemented for the `tools/pict-gen` opcode slice with co-located tests,
  fixture, citations, validation log, parser error assertions, pixel state
  assertions, and `fuzz-pict` coverage.
- Phase 4.6 PICT prerequisites backfilled 2026-06-04 after adjudication of
  `docs/blockers/resolved/phase-4.6-missing-pict-prereqs.md`: `tools/pict-gen/`
  now emits the initial PICT 2 fixture slice, `fuzz-pict` is wired through
  CMake/libFuzzer, and the local gate suite runs the generator and fuzzer.
- Phase 4.5 completed 2026-06-04 by PR #13: QuickDraw `CopyBits` and
  `CopyMask` implemented with 32-bit `PixMap` modeling, co-located tests,
  fixture, citations, validation log, mode coverage, scaling, masks, and pixel
  state assertions.
- Phase 4.4 completed 2026-06-04 by PR #12: QuickDraw region and clipping
  traps implemented with co-located tests, fixture, citations, validation log,
  and region-structure assertions.
- Phase 4.3 completed 2026-06-04 by PR #11: QuickDraw line and pen-location
  traps implemented with co-located tests, fixture, citations, validation log,
  and scanline state assertions.
- Phase 4.2 completed 2026-06-04 by PR #10: QuickDraw rectangle drawing and
  basic/RGB color traps implemented with co-located tests, fixture, citations,
  validation log, and pixel-level golden assertions.
- Phase 4.1 completed 2026-06-04 by PR #9: QuickDraw init and port-state traps
  implemented with co-located tests, fixture, citations, and validation log.
- Phase 4 missing-state blocker adjudicated on 2026-06-04 with
  `**Resume:** yes`.

## Current objective

Execute Phase 5: Memory/Resource/Sound-stub/misc Toolbox.

Required traps and routines:

- `NewHandle`
- `DisposeHandle`
- `GetHandleSize`
- `SetHandleSize`
- `HLock`
- `HUnlock`
- `HGetState`
- `HSetState`
- `NewPtr`
- `DisposePtr`
- `GetResource`
- `ReleaseResource`
- `GetResourceSizeOnDisk`
- `DetachResource`
- `TickCount`
- `GetDateTime`
- `Delay`
- `LMGetTicks`
- Silent Sound Manager stub

Required artifacts:

- Co-located implementation, test, fixture, and citation files.
- Public-source citations recorded before implementation in
  `docs/clean-room-sources.md`.
- Validation log at `docs/phase-5-validation.md`.
- Flying Toasters end-to-end checkpoint evidence, or blocker if required
  maintainer-manual artifacts are absent.
- Local gate suite and required GitHub check green before merge.

## Blocked

- 2026-06-06: Maintainer supplied the local Flying Toasters module and it was
  copied only into ignored `.nightfall/` scratch, but the current repository
  lacks a real-module execution runner/API for the required 10-minute checkpoint.
  Blocker adjudicated with `**Resume:** yes` for scoped backfill:
  `docs/blockers/resolved/phase-5-missing-real-module-runner.md`.
- 2026-06-06: Resource fork parsing and a bounded Musashi-backed `tbtrace`
  runner were backfilled, but identifying the After Dark graphics-module
  lifecycle ABI exposed Berkeley Systems sample source/header material in web
  search output. Work on that affected subsystem is stopped pending
  adjudication:
  `docs/blockers/resolved/phase-5-after-dark-lifecycle-contamination.md`.
  Maintainer marked the blocker resolved for handoff and requested this session
  cease all work because protected-code material entered the session.
- 2026-06-06: Fresh-session Phase 5 backfill added the missing C ABI surface and
  kept local gates green, but the real-module checkpoint still cannot honestly
  claim a 10-minute lifecycle run because the repository has no clean sanctioned
  After Dark graphics-module lifecycle ABI artifact. Blocker opened:
  `docs/blockers/phase-5-clean-lifecycle-abi-needed.md`.
