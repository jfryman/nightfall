# QuickDraw line drawing

This unit implements the Phase 4.3 line and pen-location subset on top of the
modeled current graphics port:

- `MoveTo`
- `Move`
- `LineTo`
- `Line`

Clean-room citations are recorded before implementation in
`docs/clean-room-sources.md`, section "QuickDraw Line Drawing".

The software model uses a deterministic integer line walk so tests can assert
scanline and diagonal pixels. This is a testable model for Nightfall's current
software framebuffer, not a claim about the exact classic QuickDraw ROM
rasterizer.
