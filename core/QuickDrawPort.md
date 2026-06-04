# QuickDraw port state

This unit implements the Phase 4.1 QuickDraw init and port-state subset:
`InitGraf`, `OpenPort`, `ClosePort`, `SetPort`, `GetPort`, and `SetPortBits`.

The implementation is intentionally an internal C++ model and does not change
the frozen public C ABI. Later Phase 4 drawing routines can build on this
modeled current-port, bitmap, region, pen, and QuickDraw-global state.

Clean-room citations are recorded before implementation in
`docs/clean-room-sources.md`, section "QuickDraw Port State". The modeled
behavior is limited to the public facts cited there:

- `InitGraf` initializes QuickDraw globals, including `thePort`, standard
  patterns, `screenBits`, and `randSeed`.
- `OpenPort` initializes a basic graphics port, allocates modeled visible and
  clipping regions, initializes documented drawing fields, and makes the port
  current.
- `ClosePort` releases the modeled visible and clipping regions.
- `SetPort` changes the modeled current port.
- `GetPort` returns the modeled current port.
- `SetPortBits` replaces the current port's `BitMap`.
- Trap words come from Apple's `Inside Macintosh X-Ref`.

The fixture lists the six Phase 4.1 trap words in the order exercised by the
dispatcher test. It is not a real module and does not encode stack or register
arguments; it exists to keep trap coverage co-located with the implementation.
