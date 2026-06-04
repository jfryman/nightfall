# Phase 4.2 Validation — QuickDraw rectangle operations

## Scope

Phase 4.2 implements rectangle drawing and foreground/background color state on
top of the Phase 4.1 modeled current graphics port:

- `FillRect`
- `EraseRect`
- `FrameRect`
- `PaintRect`
- `InvertRect`
- `BackColor`
- `ForeColor`
- `RGBBackColor`
- `RGBForeColor`

## Clean-room citations

Public sources were recorded before implementation in
`docs/clean-room-sources.md`, section "QuickDraw Rectangle Drawing".

The implementation uses:

- Apple Computer, *Inside Macintosh: Imaging With QuickDraw*, Chapter 3,
  "QuickDraw Drawing Reference", "Drawing Rectangles".
- Apple Computer, *Inside Macintosh: Imaging With QuickDraw*, Chapter 3,
  "The Eight Basic QuickDraw Colors".
- Apple Computer, *Inside Macintosh: Imaging With QuickDraw*, Chapter 4,
  "Drawing With Color QuickDraw Colors".
- Apple Computer, *Inside Macintosh X-Ref*, "System Traps", for the rectangle
  and color trap words used by the Phase 4.2 fixture.

## Artifacts

- `core/QuickDrawRect.cpp`
- `core/QuickDrawRect.test.cpp`
- `core/QuickDrawRect.fixture.s`
- `core/QuickDrawRect.md`
- Shared Phase 4 QuickDraw model declarations in `core/QuickDrawPort.h`

## Assertions

The Phase 4.2 tests assert state beyond framebuffer presence:

- `ForeColor` and `BackColor` update the current port's basic color state and
  modeled draw pixels.
- `RGBForeColor` and `RGBBackColor` store requested RGB values and direct 32-bit
  modeled pixels.
- `PaintRect` fills the rectangle interior and preserves pen location.
- `EraseRect` fills only the target rectangle with the background pixel.
- `FillRect` applies the caller pattern with deterministic `patCopy` behavior.
- `FrameRect` draws only the rectangle outline and leaves the interior unchanged.
- `InvertRect` reverses modeled direct pixels in the requested rectangle.
- Rectangle drawing rejects calls when no open current port exists.

Pixel-level assertions serve as the Phase 4.2 golden surface for the current
software model.

## Verification

Command:

```sh
scripts/ci/run-local.sh
```

Result:

```text
PASS build-and-tests
PASS sanitizers
PASS nflint good fixture
PASS nflint printf fixture
PASS nflint forbidden-source fixture
PASS nflint core
PASS boundary good fixture
PASS boundary bad fixture
PASS boundary core
PASS tbcover good fixture
PASS tbcover missing doc fixture
PASS tbcover core
PASS asm good fixture
PASS asm bad fixture
PASS asm core fixture
PASS asm sanitizer fixture
PASS depcheck good fixture
PASS depcheck missing policy fixture
PASS depcheck placeholder fixture
PASS depcheck project versions
PASS trace schema good fixture
PASS trace schema missing event fixture
PASS abi guard good fixture
PASS abi guard bad fixture
PASS sanitizer meta good fixture
PASS sanitizer meta bad fixture
PASS local gate suite
```

## Decision

Phase 4.2 is complete. Proceed to Phase 4.3 line and pen-state operations.
