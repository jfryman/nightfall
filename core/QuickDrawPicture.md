# QuickDrawPicture

Phase 4.6 PICT 2 stream handling and `DrawPicture` playback.

The implementation validates and plays back the first opcode slice that
`tools/pict-gen/` emits: version/header, RGB foreground/background colors,
lines, rectangle drawing operations, NOP, and end-of-picture. `DrawPicture`
maps the PICT frame into the supplied destination rectangle and replays those
operations into the current modeled graphics port.

Clean-room sources are recorded before implementation in
`docs/clean-room-sources.md` under "QuickDraw PICT Streams".
