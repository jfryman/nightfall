# QuickDraw rectangle drawing

This unit implements the Phase 4.2 rectangle and color subset on top of the
Phase 4.1 modeled current graphics port.

Implemented routines:

- `FrameRect`
- `PaintRect`
- `FillRect`
- `EraseRect`
- `InvertRect`
- `ForeColor`
- `BackColor`
- `RGBForeColor`
- `RGBBackColor`

Clean-room citations are recorded before implementation in
`docs/clean-room-sources.md`, section "QuickDraw Rectangle Drawing".

The model stores a bounded software pixel buffer for each modeled graphics port.
Tests use pixel-level assertions against that buffer as the Phase 4.2 golden
surface. The model is intentionally small and deterministic; it is not yet the
final 640x480 framebuffer path used by later Metal integration.
