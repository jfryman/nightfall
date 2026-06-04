# Toolbox Coverage

This coverage snapshot reflects the co-located Toolbox implementation, test,
fixture, and citation files present after Phase 4.1. The mechanical coverage
gate is `scripts/ci/tbcover.py`; `scripts/ci/run-local.sh` passed with
`PASS tbcover core`.

| Subsystem | Unit | Status | Evidence |
| --- | --- | --- | --- |
| QuickDraw | Port state (`InitGraf`, `OpenPort`, `ClosePort`, `SetPort`, `GetPort`, `SetPortBits`) | Implemented | `core/QuickDrawPort.cpp`, `core/QuickDrawPort.test.cpp`, `core/QuickDrawPort.fixture.s`, `core/QuickDrawPort.md` |
| QuickDraw | Rectangle drawing (`FillRect`, `EraseRect`, `FrameRect`, `PaintRect`, `InvertRect`, `BackColor`, `ForeColor`, `RGBBackColor`, `RGBForeColor`) | Implemented | `core/QuickDrawRect.cpp`, `core/QuickDrawRect.test.cpp`, `core/QuickDrawRect.fixture.s`, `core/QuickDrawRect.md` |

## Pending QuickDraw Phase 4 Units

- Phase 4.3 line and pen movement operations.
- Phase 4.4 region operations.
- Phase 4.5 bit and pixel map transfer operations.
- Phase 4.6 PICT interpreter operations.
