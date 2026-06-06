# Toolbox Coverage

This coverage snapshot reflects the co-located Toolbox implementation, test,
fixture, and citation files present during Phase 5. The mechanical coverage
gate is `scripts/ci/tbcover.py`; `scripts/ci/run-local.sh` passed with
`PASS tbcover core`.

| Subsystem | Unit | Status | Evidence |
| --- | --- | --- | --- |
| QuickDraw | Port state (`InitGraf`, `OpenPort`, `ClosePort`, `SetPort`, `GetPort`, `SetPortBits`) | Implemented | `core/QuickDrawPort.cpp`, `core/QuickDrawPort.test.cpp`, `core/QuickDrawPort.fixture.s`, `core/QuickDrawPort.md` |
| QuickDraw | Rectangle drawing (`FillRect`, `EraseRect`, `FrameRect`, `PaintRect`, `InvertRect`, `BackColor`, `ForeColor`, `RGBBackColor`, `RGBForeColor`) | Implemented | `core/QuickDrawRect.cpp`, `core/QuickDrawRect.test.cpp`, `core/QuickDrawRect.fixture.s`, `core/QuickDrawRect.md` |
| QuickDraw | Line drawing (`MoveTo`, `Move`, `LineTo`, `Line`) | Implemented | `core/QuickDrawLine.cpp`, `core/QuickDrawLine.test.cpp`, `core/QuickDrawLine.fixture.s`, `core/QuickDrawLine.md` |
| QuickDraw | Region operations (`NewRgn`, `DisposeRgn`, `SetRectRgn`, `RectRgn`, `OffsetRgn`, `CopyRgn`, `UnionRgn`, `SectRgn`, `DiffRgn`, `EmptyRgn`, `EqualRgn`, `PtInRgn`, `SetClip`, `GetClip`) | Implemented | `core/QuickDrawRegion.cpp`, `core/QuickDrawRegion.test.cpp`, `core/QuickDrawRegion.fixture.s`, `core/QuickDrawRegion.md` |
| QuickDraw | Bit transfer (`CopyBits`, `CopyMask`) | Implemented | `core/QuickDrawBitTransfer.cpp`, `core/QuickDrawBitTransfer.test.cpp`, `core/QuickDrawBitTransfer.fixture.s`, `core/QuickDrawBitTransfer.md` |
| QuickDraw | PICT interpreter (`DrawPicture`) | Implemented | `core/QuickDrawPicture.cpp`, `core/QuickDrawPicture.test.cpp`, `core/QuickDrawPicture.fixture.s`, `core/QuickDrawPicture.md`, `core/QuickDrawPicture.fuzz.cpp` |
| Toolbox Managers | Memory/Resource/Sound-stub/misc (`NewHandle`, `DisposeHandle`, `GetHandleSize`, `SetHandleSize`, `HLock`, `HUnlock`, `HGetState`, `HSetState`, `NewPtr`, `DisposePtr`, `GetResource`, `ReleaseResource`, `GetResourceSizeOnDisk`, `DetachResource`, `TickCount`, `GetDateTime`, `Delay`, `LMGetTicks`, silent Sound Manager) | Implemented | `core/ToolboxManagers.cpp`, `core/ToolboxManagers.test.cpp`, `core/ToolboxManagers.fixture.s`, `core/ToolboxManagers.md` |

## Pending Phase 5 Checkpoint

- Maintainer-manual Flying Toasters end-to-end 10-minute run with zero
  unimplemented-trap warnings, or blocker if the artifact is absent.
