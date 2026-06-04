# Nightfall — Current Phase

**Current phase:** 4.6
**Status:** In progress
**Started:** 2026-06-04
**Timebox:** 18 cycles / 24 active hours
**Cycle count:** 0 / 18
**Active hours:** 0 / 24
**Next expected deliverable:** docs/phase-4.6-validation.md
**Agent(s) assigned:** codex-gpt-5
**Last blocker:** docs/blockers/resolved/phase-4.6-missing-pict-prereqs.md
**Run status:** in progress

## Recent completions

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

Execute Phase 4.6: QuickDraw PICT interpreter operations.

Required traps:

- `DrawPicture`

Required artifacts:

- Co-located implementation, test, fixture, and citation files.
- Public-source citations recorded before implementation in
  `docs/clean-room-sources.md`.
- PICT 2 opcode coverage matching `tools/pict-gen` coverage.
- `fuzz-pict` mandatory for this subphase.
- Picture decoding, pixel state, and parser error assertions.
- Validation log at `docs/phase-4.6-validation.md`.
- Local gate suite and required GitHub check green before merge.
