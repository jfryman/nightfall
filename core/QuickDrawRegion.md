# QuickDraw region operations

This unit implements the Phase 4.4 modeled region subset:

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

Clean-room citations are recorded before implementation in
`docs/clean-room-sources.md`, section "QuickDraw Region Operations".

The software model stores region structure as a bounded 64x64 pixel set. That is
enough for deterministic state assertions in Phase 4.4 without committing to the
classic variable-sized region record encoding before the Memory Manager-backed
Toolbox model lands.
