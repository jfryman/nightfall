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

## QuickDraw Line Drawing

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing", "Drawing Lines", page 3-17, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-60.html`
- **Consulted for:** `MoveTo` and `Move` place the graphics pen in local
  coordinates; `LineTo` and `Line` draw from the current pen location to a new
  point; line drawing updates the pen location.
- **Implemented from this source:** Phase 4.3 line operations update the modeled
  current port's pen location and draw into the modeled software pixels.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing", "The Graphics Pen", page 3-4, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-59.html`
- **Consulted for:** The graphics pen location is stored in the current graphics
  port, the pen draws below and to the right of its defining point, and `MoveTo`
  / `Move` change the pen location without drawing.
- **Implemented from this source:** Phase 4.3 tests assert pen location state
  before and after move and line operations.

- **Source:** J. E. Bresenham, "Algorithm for Computer Control of a Digital
  Plotter", *IBM Systems Journal*, Vol. 4, No. 1, 1965.
- **Consulted for:** Deterministic integer rasterization of a line segment on a
  software pixel grid.
- **Implemented from this source:** Phase 4.3 uses an integer line walk to mark
  modeled pixels between the old and new pen locations. This is a deterministic
  software model for tests, not an assertion that classic QuickDraw's internal
  rasterizer used this exact implementation.

- **Source:** Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-76, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Trap words for `LineTo` (`A891`), `Line` (`A892`),
  `MoveTo` (`A893`), and `Move` (`A894`).
- **Implemented from this source:** Phase 4.3 QuickDraw line trap-word constants
  for the fixture and internal dispatcher tests.

## QuickDraw Region Operations

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", "Creating and Managing Regions",
  pages 3-83 to 3-94, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-131.html`
- **Consulted for:** Region allocation and disposal, rectangular region
  construction, moving regions, copying regions, set operations, point
  containment, equality, and empty-region behavior.
- **Implemented from this source:** Phase 4.4 models regions as bounded pixel
  sets so the implemented traps can update region structure deterministically.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", `NewRgn`, page 3-83, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-132.html`
- **Consulted for:** `NewRgn` creates the only new region handle, initializes it
  to the empty region `(0,0,0,0)`, and other routines alter existing regions.
- **Implemented from this source:** Phase 4.4 `NewRgn` allocates a modeled
  region handle initialized to an empty region.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", `SetRectRgn` and `RectRgn`, pages
  3-87 to 3-88, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-138.html` and
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-139.html`
- **Consulted for:** Rectangular regions replace prior region structure, and an
  empty rectangle creates the empty region `(0,0,0,0)`.
- **Implemented from this source:** Phase 4.4 `SetRectRgn` and `RectRgn` replace
  modeled region structure with a rectangular pixel set or an empty region.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", `SectRgn`, `UnionRgn`, `DiffRgn`,
  `PtInRgn`, `EqualRgn`, and `EmptyRgn`, pages 3-90 to 3-94, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-142.html`,
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-143.html`,
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-144.html`,
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-146.html`,
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-148.html`, and
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-149.html`
- **Consulted for:** Region intersection, union, difference, point containment,
  equality, and empty-region semantics.
- **Implemented from this source:** Phase 4.4 region operations update and query
  the modeled pixel-set representation.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 2, "Basic QuickDraw Reference", `GetClip` and `SetClip`, pages 2-47
  to 2-48, mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/QuickDraw/QuickDraw-44.html`
- **Consulted for:** `SetClip` copies the specified region into the current
  port's clipping region without changing the source handle; `GetClip` copies
  the current port clipping region into the supplied region handle.
- **Implemented from this source:** Phase 4.4 `SetClip` and `GetClip` copy
  modeled region structures between region handles and the current port.

- **Source:** Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-77, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Trap words for `SetClip` (`A879`), `GetClip` (`A87A`),
  `NewRgn` (`A8D8`), `DisposeRgn` (`A8D9`), `CopyRgn` (`A8DC`),
  `SetRectRgn` (`A8DE`), `RectRgn` (`A8DF`), `OffsetRgn` (`A8E0`),
  `SectRgn` (`A8E4`), `UnionRgn` (`A8E5`), `DiffRgn` (`A8E6`),
  `PtInRgn` (`A8E8`), `EmptyRgn` (`A8E2`), and `EqualRgn` (`A8E3`).
- **Implemented from this source:** Phase 4.4 QuickDraw region and clipping
  trap-word constants for fixtures and internal dispatcher tests.

## QuickDraw Bit Transfer

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", "Copying Images", page 3-112,
  mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-165.html`
- **Consulted for:** `CopyBits` copies with source modes, clipping, and resizing;
  `CopyMask` masks copy areas; both can operate on bitmaps or pixel maps.
- **Implemented from this source:** Phase 4.5 models bounded bitmap/pixel-map
  transfer between modeled graphics ports.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", `CopyBits`, pages 3-113 to 3-115,
  mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-166.html`
- **Consulted for:** `CopyBits` source and destination bitmap parameters,
  source/destination rectangles, source modes, optional mask region clipping,
  scaling to destination rectangles, and clipping to destination bounds/current
  port clipping.
- **Implemented from this source:** Phase 4.5 `CopyBits` copies modeled 32-bit
  pixels with nearest-neighbor scaling, basic source modes, optional mask-region
  clipping, and current-port clipping.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 3, "QuickDraw Drawing Reference", `CopyMask`, page 3-115, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-167.html`
- **Consulted for:** `CopyMask` transfers source pixels only where a supplied
  mask bitmap/pixel map permits the copy, and uses source, mask, and destination
  rectangles.
- **Implemented from this source:** Phase 4.5 `CopyMask` copies modeled pixels
  only where the modeled mask pixel is active.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 4, "Color QuickDraw", direct-device 32-bit pixel representation,
  mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-198.html`
- **Consulted for:** Phase 4's 32-bit direct-color assumption and pixel-map
  treatment for color graphics ports.
- **Implemented from this source:** Phase 4.5 adds a modeled 32-bit `PixMap`
  alongside each modeled port's `BitMap`.

- **Source:** Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-77, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Trap word for `CopyBits` (`A8EC`).
- **Implemented from this source:** Phase 4.5 `CopyBits` trap-word constant for
  fixtures and trace assertions.

- **Source:** Apple Computer, *Universal Interfaces*, `Quickdraw.h` for Mac OS 9,
  mirrored at:
  `https://www.cs.vu.nl/~eliens/research/media/lib-of-vs-libs-QTDevWin-CIncludes-Quickdraw.h.html`
- **Consulted for:** Trap word for `CopyMask` (`A817`) in the public Apple
  QuickDraw interface header.
- **Implemented from this source:** Phase 4.5 `CopyMask` trap-word constant for
  fixtures and trace assertions.

## QuickDraw PICT Streams

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 7, "Pictures", `Picture` record, mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/QuickDraw/QuickDraw-337.html`
- **Consulted for:** A `Picture` record begins with `picSize` and `picFrame`,
  followed by picture opcode data, and `picFrame` is the bounding rectangle used
  by `DrawPicture`.
- **Implemented from this source:** Phase 4.6 prerequisite parser validates the
  modeled PICT 2 record envelope before opcode coverage is considered.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Chapter 7, "Pictures", `DrawPicture`, mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/QuickDraw/QuickDraw-349.html`
- **Consulted for:** `DrawPicture` draws a picture into the current graphics
  port, using a destination rectangle to place the picture playback.
- **Implemented from this source:** Phase 4.6 `DrawPicture` replays the covered
  PICT 2 opcode slice into the modeled current graphics port, mapping picture
  frame coordinates into the supplied destination rectangle.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Appendix A, "Picture Opcodes", mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-458.html`
- **Consulted for:** The first opcode in a picture is the version opcode,
  followed by the version number, and picture opcodes are the data consumed by
  `DrawPicture`.
- **Implemented from this source:** Phase 4.6 prerequisite generator and parser
  emit and validate the PICT 2 version prelude.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Appendix A, "A Sample Version 2 Picture", mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-463.html`
- **Consulted for:** Version 2 PICT streams begin with `VersionOp` `$0011`,
  version `$02FF`, and `HeaderOp` `$0C00` with 24 bytes of header data, and end
  with `OpEndPic` `$00FF`.
- **Implemented from this source:** `tools/pict-gen/` emits the modeled version
  2 prelude/header/end sequence, and `QuickDrawPicture` validates it.

- **Source:** Apple Computer, *Inside Macintosh: Imaging With QuickDraw*,
  Appendix A, "Opcodes in Pictures", Table A-2, mirrored at:
  `https://dev.os9.ca/techpubs/mac/QuickDraw/QuickDraw-461.html`
- **Consulted for:** PICT 2 opcodes and data lengths for NOP, RGB foreground and
  background colors, line, rectangle drawing operations, version/header, and
  end-of-picture.
- **Implemented from this source:** Phase 4.6 prerequisite `tools/pict-gen/`
  coverage and `QuickDrawPicture` parser coverage for the initial opcode set
  that `DrawPicture` will consume.

- **Source:** Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-77, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Trap word for `DrawPicture` (`A8F6`).
- **Implemented from this source:** Phase 4.6 `DrawPicture` trap-word constant
  for the fixture and trace assertions.

## Phase 5 Toolbox Managers

- **Source:** Apple Computer, *Inside Macintosh: Memory*, Chapter 2, "Memory
  Manager Reference", `NewHandle`, mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-67.html`
- **Consulted for:** `NewHandle` allocates a relocatable memory block of the
  requested logical size and returns a handle; newly allocated handles are
  unlocked and unpurgeable.
- **Implemented from this source:** Phase 5 models handles as relocatable blocks
  with logical size, data address, and state bits.

- **Source:** Apple Computer, *Inside Macintosh: Memory*, Chapter 2, "Memory
  Manager Reference", `DisposeHandle`, mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-73.html`
- **Consulted for:** `DisposeHandle` releases the memory occupied by a
  relocatable block and invalidates subsequent use of that handle.
- **Implemented from this source:** Phase 5 invalidates modeled handles on
  disposal and rejects stale handle operations.

- **Source:** Apple Computer, *Inside Macintosh: Memory*, Chapter 2, "Memory
  Manager Reference", "Allocating and Releasing Nonrelocatable Blocks of
  Memory", mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-74.html`
- **Consulted for:** `NewPtr` allocates a nonrelocatable block and
  `DisposePtr` frees nonrelocatable blocks allocated by the Memory Manager.
- **Implemented from this source:** Phase 5 models fixed pointer blocks with
  stable addresses and explicit disposal.

- **Source:** Apple Computer, *Inside Macintosh: Memory*, Chapter 2, "Memory
  Manager Reference", "Changing the Sizes of Relocatable and Nonrelocatable
  Blocks", mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-80.html`
- **Consulted for:** `GetHandleSize` and `SetHandleSize` query and change the
  logical size of a relocatable block.
- **Implemented from this source:** Phase 5 tracks modeled handle logical sizes
  and permits deterministic resizing.

- **Source:** Apple Computer, *Inside Macintosh: Memory*, Chapter 2, "Memory
  Manager Reference", `HGetState`, `HSetState`, `HLock`, and `HUnlock`,
  mirrored at:
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-86.html`,
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-87.html`,
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-88.html`, and
  `https://leopard-adc.pepas.com/documentation/mac/Memory/Memory-89.html`
- **Consulted for:** `HGetState` returns master-pointer state flags, including
  resource, purgeable, and locked bits; `HSetState` restores those flags;
  `HLock` and `HUnlock` set and clear the locked bit.
- **Implemented from this source:** Phase 5 preserves the documented state bits
  in modeled handles, with bit 7 used for locking and bit 5 for Resource Manager
  ownership.

- **Source:** Apple Computer, *Inside Macintosh: More Macintosh Toolbox*,
  Chapter 1, "Resource Manager Reference", `GetResource`, mirrored at:
  `https://dev.os9.ca/techpubs/mac/MoreToolbox/MoreToolbox-50.html`
- **Consulted for:** `GetResource` searches resource maps by type and ID and
  returns a handle to the resource data, reading it into memory when necessary.
- **Implemented from this source:** Phase 5 provides a deterministic in-memory
  resource map for tests and fixtures, keyed by type and ID, returning modeled
  Memory Manager handles.

- **Source:** Apple Computer, *Inside Macintosh: More Macintosh Toolbox*,
  Chapter 1, "Resource Manager", "Releasing and Detaching Resources", mirrored
  at:
  `https://dev.os9.ca/techpubs/mac/MoreToolbox/MoreToolbox-19.html`
- **Consulted for:** `ReleaseResource` releases memory associated with a
  resource and invalidates the application's handle; `DetachResource` removes a
  resource handle from Resource Manager ownership while leaving the application
  handle usable.
- **Implemented from this source:** Phase 5 releases modeled resource handles
  and detaches resource ownership without freeing the underlying modeled handle.

- **Source:** Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-77, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Trap words for the Phase 5 Memory Manager, Resource
  Manager, time, and Sound Manager routines covered by this implementation.
- **Implemented from this source:** Phase 5 trap-word constants for fixtures and
  trace assertions.

- **Source:** Apple Computer, *Inside Macintosh: Sound*, Chapter 2, "Sound
  Manager", "Managing Sound Channels", mirrored at:
  `https://dev.os9.ca/techpubs/mac/Sound/Sound-51.html`
- **Consulted for:** `SndNewChannel` allocates a sound channel and command
  queue; `SndDoCommand` queues commands on a sound channel. The plan requires
  Sound Manager to be stubbed silent in Phase 5.
- **Implemented from this source:** Phase 5 allocates modeled silent sound
  channels, records queued commands, and performs no audio output.

- **Source:** Nightfall plan Q1 timing model in `nightfall-plan-macos.md`,
  together with Apple Computer, *Inside Macintosh X-Ref*, "System Traps", pages
  63-77, preserved at:
  `https://vintageapple.org/macprogramming/pdf/Inside_Macintosh_X-Ref_1988.pdf`
- **Consulted for:** Nightfall uses a virtual clock; `TickCount` reads that
  virtual counter. X-Ref records trap words for `TickCount`, `Delay`, and the
  date/time routines.
- **Implemented from this source:** Phase 5 exposes deterministic
  `TickCount`, `LMGetTicks`, `Delay`, `GetDateTime`, and virtual date/time
  state for headless tests.

## Phase 5 Resource Fork Runner Backfill

- **Source:** Apple Computer, *Inside Macintosh: More Macintosh Toolbox*,
  Chapter 1, "Resource Manager Reference", "Resource File Format", mirrored at:
  `https://dev.os9.ca/techpubs/mac/MoreToolbox/MoreToolbox-99.html`
- **Consulted for:** Resource forks contain a resource data area and a
  resource map; the map contains a resource type list and per-type reference
  lists; resource data is addressed from the data area by offsets recorded in
  reference entries.
- **Implemented from this source:** `core/ResourceFork.cpp` validates the
  resource fork header, map offsets, type list, reference lists, optional
  resource names, and length-prefixed resource payloads before exposing a
  type/ID index to the local runner.

- **Source:** Apple Computer, *Inside Macintosh: More Macintosh Toolbox*,
  Chapter 1, "Resource Manager Reference", "Resource File Format", figure and
  field descriptions, mirrored at:
  `https://dev.os9.ca/techpubs/mac/MoreToolbox/MoreToolbox-99.html`
- **Consulted for:** Type entries store the four-character resource type, a
  count-minus-one, and the reference-list offset; reference entries store the
  resource ID, optional name-list offset, attributes, and three-byte data
  offset.
- **Implemented from this source:** `core/ResourceFork.cpp` decodes those
  big-endian fields directly and rejects truncated or out-of-range structures.

- **Source:** Karl Stenerud, Musashi `readme.txt`, "Basic Configuration", from
  pinned commit `313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd` at:
  `https://github.com/kstenerud/Musashi`
- **Consulted for:** Musashi hosts provide `m68k_read_memory_*` and
  `m68k_write_memory_*` callbacks, call `m68k_pulse_reset`, and run instructions
  through `m68k_execute`.
- **Implemented from this source:** `core/M68KRuntime.cpp` supplies bounded
  big-endian emulated-memory callbacks, seeds reset vectors, and executes a
  68000 program image under a finite cycle budget.

- **Source:** Karl Stenerud, Musashi `m68kconf.h`, instruction hook
  configuration, from pinned commit
  `313ebf1bd9f4d0d93341eb5ce21fd8a119e9dbdd` at:
  `https://github.com/kstenerud/Musashi`
- **Consulted for:** `M68K_INSTRUCTION_HOOK` with
  `M68K_OPT_SPECIFY_HANDLER` calls a host-specified instruction callback before
  each instruction.
- **Implemented from this source:** `core/M68KRuntime.cpp` uses the instruction
  hook to record A-line Toolbox trap words and skip them before Musashi raises
  the 1010 exception.
