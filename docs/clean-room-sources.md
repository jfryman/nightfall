# Clean-Room Sources

Record the public specifications consulted before implementing any trap, host
routine, or emulation behavior.

Rules:

- Cite before implementation.
- Prefer specific page, section, or chapter references.
- Keep entries scoped to the subsystem they justify.
- If any output appears influenced by a forbidden source, stop and follow
  `docs/contamination-response.md`.

## Bootstrap status

## Trap Dispatch

- **Source:** Apple Computer, *A/UX Toolbox: Macintosh ROM Interface*, Appendix C
  "Implementation Notes", "A-line traps", pages C-3 to C-5, preserved at
  Bitsavers:
  `https://bitsavers.computerhistory.org/pdf/apple/mac/a_ux/aux_3.0/AUX_3.0_AUX_Toolbox_Macintosh_ROM_Interface_1992.pdf`
- **Consulted for:** The limited bootstrap facts that A-line traps are MC680x0
  opcodes in the range `0xA000` to `0xAFFF`, and that Macintosh environments use
  dispatch tables to route A-line traps to ROM or patched/user routines.
- **Implemented from this source:** `nf_trap_is_aline`,
  `nf_context_register_trap`, and `nf_context_dispatch_trap` implement only a
  Nightfall host-handler dispatch scaffold. No Toolbox routine behavior is
  implemented from this source.

## 68k Fixture Runner

- **Source:** Motorola, *M68000 Family Programmer's Reference Manual*, "RTS
  Return from Subroutine", page 4-169, and instruction format summary pages
  8-13 to 8-14, hosted by NXP:
  `https://www.nxp.com/docs/en/reference-manual/M68000PRM.pdf`
- **Consulted for:** The `RTS` instruction format word used by bootstrap
  fixtures.
- **Implemented from this source:** `nf_context_execute_fixture` recognizes a
  big-endian stream of 16-bit words, dispatches A-line trap words through the
  Nightfall trap scaffold, and treats `RTS` as the stop marker for the bootstrap
  fixture runner. It does not implement stack effects, registers, or general
  MC68000 instruction semantics.

## QuickDraw Port State

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 2, "Basic QuickDraw Reference", "InitGraf", page 2-34, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-30.html`
- **Consulted for:** `InitGraf` initializes QuickDraw globals, stores the
  `thePort` global pointer, sets `thePort` to `NIL`, initializes the standard
  white/black/gray patterns, initializes `screenBits`, and initializes
  `randSeed` to `1`.
- **Implemented from this source:** Phase 4.1 `InitGraf` stores a modeled
  QuickDraw global pointer and initializes Nightfall's modeled QuickDraw globals.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 2, "Basic QuickDraw Reference", "OpenPort", page 2-35, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-32.html`
- **Consulted for:** `OpenPort` initializes a `GrafPort`, initializes visible
  and clipping regions, gives the port the documented initial drawing state, and
  makes it the current port by calling `SetPort`.
- **Implemented from this source:** Phase 4.1 `OpenPort` initializes the modeled
  `GrafPort` fields needed by later QuickDraw drawing phases and makes that port
  current.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 2, "Basic QuickDraw Reference", "ClosePort", page 2-36, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-34.html`
- **Consulted for:** `ClosePort` releases a basic graphics port's visible and
  clipping regions.
- **Implemented from this source:** Phase 4.1 `ClosePort` marks the modeled
  visible and clipping regions released while preserving the `GrafPort` record
  itself.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 2, "Basic QuickDraw Reference", "GetPort", page 2-39, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-36.html`
- **Consulted for:** `GetPort` returns the current graphics port pointer in the
  caller-provided parameter.
- **Implemented from this source:** Phase 4.1 `GetPort` returns Nightfall's
  modeled current port address.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 2, "Basic QuickDraw Reference", "SetPort", page 2-39, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-37.html`
- **Consulted for:** `SetPort` sets the current graphics port; drawing routines
  use the bitmap and local coordinate system of that current port.
- **Implemented from this source:** Phase 4.1 `SetPort` updates Nightfall's
  modeled current port pointer.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 2, "Basic QuickDraw Reference", "SetPortBits", page 2-42, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-47.html`
- **Consulted for:** `SetPortBits` replaces the `portBits` field of the current
  basic graphics port with a previously prepared `BitMap`.
- **Implemented from this source:** Phase 4.1 `SetPortBits` replaces the modeled
  current port's `BitMap`.

- **Source:** Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-76, especially page 76 for QuickDraw trap words, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Trap words for `InitPort` (`A86D`), `InitGraf` (`A86E`),
  `OpenPort` (`A86F`), `SetPort` (`A873`), `GetPort` (`A874`), `SetPBits` /
  `SetPortBits` (`A875`), and `ClosePort` (`A87D`).
- **Implemented from this source:** Phase 4.1 QuickDraw trap-word constants for
  the fixture and internal dispatcher tests.

## QuickDraw Rectangle Drawing

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", "Drawing Rectangles", pages 3-26
  to 3-30, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-96.html`
- **Consulted for:** `FrameRect` outlines a rectangle with the current pen;
  `PaintRect` fills a rectangle with the pen pattern and mode; `FillRect` fills
  with a caller-supplied pattern using `patCopy`; `EraseRect` fills with the
  current port background pattern; `InvertRect` reverses pixels in the rectangle.
- **Implemented from this source:** Phase 4.2 rectangle operations update the
  modeled current port's software pixels according to those operation roles.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing", "The Eight Basic QuickDraw Colors", page
  3-12, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-59.html`
- **Consulted for:** Basic QuickDraw uses `fgColor` and `bkColor` fields for
  foreground and background colors, defaults to black foreground and white
  background, and `ForeColor` / `BackColor` update those fields using the eight
  predefined basic colors.
- **Implemented from this source:** Phase 4.2 `ForeColor` and `BackColor`
  update the modeled current port foreground and background colors.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 4, "Color QuickDraw Reference", "Drawing With Color QuickDraw Colors",
  pages 4-61 to 4-63, mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/QuickDraw/QuickDraw-220.html`
- **Consulted for:** `RGBForeColor` sets the foreground color and
  `RGBBackColor` sets the background color; both operate for basic graphics
  ports in System 7.
- **Implemented from this source:** Phase 4.2 `RGBForeColor` and
  `RGBBackColor` store the requested RGB values and modeled direct 32-bit pixel
  colors in the current port.

- **Source:** Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-76, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Trap words for `BackColor` (`A863`), `ForeColor` (`A862`),
  `FrameRect` (`A8A1`), `PaintRect` (`A8A2`), `EraseRect` (`A8A3`),
  `InvertRect` (`A8A4`), `FillRect` (`A8A5`), `RGBForeColor` (`AA14`), and
  `RGBBackColor` (`AA15`).
- **Implemented from this source:** Phase 4.2 QuickDraw rectangle and color
  trap-word constants for the fixture and internal dispatcher tests.
