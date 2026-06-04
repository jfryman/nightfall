# Phase 4.4 Validation — QuickDraw Regions

## Scope

Phase 4.4 implements the QuickDraw region and clipping subset:

- `NewRgn`
- `DisposeRgn`
- `SetRectRgn`
- `RectRgn`
- `OffsetRgn`
- `CopyRgn`
- `UnionRgn`
- `SectRgn`
- `DiffRgn`
- `EmptyRgn`
- `EqualRgn`
- `PtInRgn`
- `SetClip`
- `GetClip`

The implementation is co-located with its test, fixture, and citation file:

- `core/QuickDrawRegion.cpp`
- `core/QuickDrawRegion.test.cpp`
- `core/QuickDrawRegion.fixture.s`
- `core/QuickDrawRegion.md`

## Clean-Room Sources

Citations were recorded before implementation in `docs/clean-room-sources.md`,
section "QuickDraw Region Operations". The implementation uses Inside Macintosh
for region allocation, rectangular construction, movement, set operations, point
containment, equality, empty-region behavior, clipping behavior, and X-Ref trap
words.

## Assertions

- `NewRgn` creates an allocated empty modeled region.
- `DisposeRgn` releases a modeled region and rejects stale handles.
- `SetRectRgn` and `RectRgn` replace prior region structure and produce empty
  regions for empty rectangles.
- `OffsetRgn` moves a modeled region while preserving shape.
- `CopyRgn` creates an independent duplicate.
- `UnionRgn`, `SectRgn`, and `DiffRgn` update destination region structure with
  pixel-set assertions.
- `EmptyRgn`, `EqualRgn`, and `PtInRgn` report region state.
- `SetClip` and `GetClip` copy region structure through the current port without
  aliasing the source handle.
- Missing or disposed handles return `NF_ERROR_INVALID_ARGUMENT`.

## Local Validation

- PASS: `scripts/ci/run-local.sh`
