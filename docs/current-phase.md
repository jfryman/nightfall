# Nightfall — Current Phase

**Current phase:** 5
**Status:** Blocked
**Started:** 2026-06-06
**Timebox:** 24 cycles / 32 active hours
**Cycle count:** 0 / 24
**Active hours:** 0 / 32
**Next expected deliverable:** docs/phase-5-validation.md
**Agent(s) assigned:** codex-gpt-5
**Last blocker:** docs/blockers/phase-5-missing-flying-toasters-artifact.md
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

- 2026-06-06: Phase 5 implementation and local gates are complete, but the
  maintainer-manual Flying Toasters 10-minute checkpoint artifact is absent.
  Open blocker:
  `docs/blockers/phase-5-missing-flying-toasters-artifact.md`.
