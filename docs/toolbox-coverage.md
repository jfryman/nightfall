# Toolbox Coverage

This coverage snapshot reflects the co-located Toolbox implementation, test,
fixture, and citation files present after Phase 4.5. The mechanical coverage
gate is `scripts/ci/tbcover.py`; `scripts/ci/run-local.sh` passed with
`PASS tbcover core`.

| Subsystem | Unit | Status | Evidence |
| --- | --- | --- | --- |
| QuickDraw | Port state (`InitGraf`, `OpenPort`, `ClosePort`, `SetPort`, `GetPort`, `SetPortBits`) | Implemented | `core/QuickDrawPort.cpp`, `core/QuickDrawPort.test.cpp`, `core/QuickDrawPort.fixture.s`, `core/QuickDrawPort.md` |
| QuickDraw | Rectangle drawing (`FillRect`, `EraseRect`, `FrameRect`, `PaintRect`, `InvertRect`, `BackColor`, `ForeColor`, `RGBBackColor`, `RGBForeColor`) | Implemented | `core/QuickDrawRect.cpp`, `core/QuickDrawRect.test.cpp`, `core/QuickDrawRect.fixture.s`, `core/QuickDrawRect.md` |
| QuickDraw | Line drawing (`MoveTo`, `Move`, `LineTo`, `Line`) | Implemented | `core/QuickDrawLine.cpp`, `core/QuickDrawLine.test.cpp`, `core/QuickDrawLine.fixture.s`, `core/QuickDrawLine.md` |
| QuickDraw | Region operations (`NewRgn`, `DisposeRgn`, `SetRectRgn`, `RectRgn`, `OffsetRgn`, `CopyRgn`, `UnionRgn`, `SectRgn`, `DiffRgn`, `EmptyRgn`, `EqualRgn`, `PtInRgn`, `SetClip`, `GetClip`) | Implemented | `core/QuickDrawRegion.cpp`, `core/QuickDrawRegion.test.cpp`, `core/QuickDrawRegion.fixture.s`, `core/QuickDrawRegion.md` |
| QuickDraw | Bit transfer (`CopyBits`, `CopyMask`) | Implemented | `core/QuickDrawBitTransfer.cpp`, `core/QuickDrawBitTransfer.test.cpp`, `core/QuickDrawBitTransfer.fixture.s`, `core/QuickDrawBitTransfer.md` |

## Pending QuickDraw Phase 4 Units

- Phase 4.6 PICT interpreter operations.
