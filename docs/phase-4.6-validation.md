# Phase 4.6 Validation — QuickDraw PICT Interpreter

## Scope

Phase 4.6 implements `DrawPicture` playback for the PICT 2 opcode slice covered
by `tools/pict-gen/` and `fuzz-pict`:

- Version 2 prelude and header
- NOP
- RGB foreground and background colors
- Line
- FrameRect
- PaintRect
- EraseRect
- InvertRect
- FillRect
- EndPic

The implementation is co-located with its test, fixture, documentation, and
fuzzer:

- `core/QuickDrawPicture.cpp`
- `core/QuickDrawPicture.test.cpp`
- `core/QuickDrawPicture.fixture.s`
- `core/QuickDrawPicture.md`
- `core/QuickDrawPicture.fuzz.cpp`

## Clean-Room Sources

Citations were recorded before implementation in `docs/clean-room-sources.md`,
section "QuickDraw PICT Streams". The implementation uses Inside Macintosh for
the `Picture` record, `DrawPicture` playback, PICT 2 version/header/end
structure, opcode values and data lengths, and Inside Macintosh X-Ref for the
`DrawPicture` trap word.

## Assertions

- `validate_pict2` preserves parser coverage and error behavior.
- `DrawPicture` replays covered PICT 2 opcodes into the current modeled graphics
  port.
- RGB foreground/background opcodes update modeled QuickDraw colors before later
  drawing operations.
- Line and rectangle opcodes update pixel state through existing QuickDraw trap
  implementations.
- Picture frame coordinates map into the supplied destination rectangle.
- Unsupported and truncated streams return parser errors.
- `DrawPicture` records the `A8F6` trap word and `fuzz-pict` exercises playback
  as well as validation.

## Local Validation

- PASS: `scripts/ci/run-local.sh`
