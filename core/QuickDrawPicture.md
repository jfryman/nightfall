# QuickDrawPicture

Phase 4.6 prerequisite recovery for PICT 2 stream handling.

The implementation intentionally stops at validation and opcode coverage. It
checks the PICT 2 envelope and the first opcode slice that `tools/pict-gen/`
emits: version/header, RGB foreground/background colors, lines, rectangle
drawing operations, NOP, and end-of-picture. `DrawPicture` playback will consume
this parser surface during the rest of Phase 4.6.

Clean-room sources are recorded before implementation in
`docs/clean-room-sources.md` under "QuickDraw PICT Streams".
