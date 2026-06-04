# Phase 4.3 Validation — QuickDraw Lines

## Scope

Phase 4.3 implements the QuickDraw line and pen-location subset:

- `MoveTo`
- `Move`
- `LineTo`
- `Line`

The implementation is co-located with its test, fixture, and citation file:

- `core/QuickDrawLine.cpp`
- `core/QuickDrawLine.test.cpp`
- `core/QuickDrawLine.fixture.s`
- `core/QuickDrawLine.md`

## Clean-Room Sources

Citations were recorded before implementation in `docs/clean-room-sources.md`,
section "QuickDraw Line Drawing". The implementation uses Inside Macintosh for
QuickDraw line and pen-location behavior, Inside Macintosh X-Ref for trap words,
and Bresenham's public 1965 paper for deterministic integer rasterization in the
Nightfall software test model.

## Assertions

- `MoveTo` and `Move` update the current port's `pn_loc` without drawing pixels.
- `LineTo` draws a horizontal scanline and updates `pn_loc`.
- `Line` draws a relative vertical scanline and updates `pn_loc`.
- Diagonal output is deterministic on the modeled software pixel grid.
- Line operations fail without an open current port.
- The QuickDraw trap-word dispatcher covers the Phase 4.3 line trap constants.

## Local Validation

- PASS: `scripts/ci/run-local.sh`
