# Phase 4.1 Validation — QuickDraw init and port state

## Scope

Phase 4.1 implements the initial QuickDraw port-state subset:

- `_InitGraf`
- `OpenPort`
- `ClosePort`
- `SetPort`
- `GetPort`
- `SetPortBits` (`_SetPBits`)

## Clean-room citations

Public sources were recorded before implementation in
`docs/clean-room-sources.md`, section "QuickDraw Port State".

The implementation uses:

- Apple Computer, *Inside Macintosh: Imaging With QuickDraw*, Chapter 2,
  "Basic QuickDraw Reference", `InitGraf`, `OpenPort`, `ClosePort`, `GetPort`,
  `SetPort`, and `SetPortBits`.
- Apple Computer, *Inside Macintosh X-Ref*, "System Traps", for the QuickDraw
  trap words used by the Phase 4.1 fixture.

## Artifacts

- `core/QuickDrawPort.cpp`
- `core/QuickDrawPort.h`
- `core/QuickDrawPort.test.cpp`
- `core/QuickDrawPort.fixture.s`
- `core/QuickDrawPort.md`

## Assertions

The Phase 4.1 tests assert state beyond framebuffer output:

- `InitGraf` initializes modeled QuickDraw globals, standard patterns,
  `screenBits`, `thePort`, and `randSeed`.
- `OpenPort` initializes a modeled `GrafPort`, visible and clipping regions,
  pen state, text state, colors, saved handles, and makes the port current.
- `SetPort` and `GetPort` preserve and report the current port while keeping
  independent port state intact.
- `SetPortBits` replaces only the current port bitmap.
- `ClosePort` releases modeled visible and clipping regions without disposing
  the port record.
- The internal Phase 4.1 trap-word dispatcher covers the six implemented trap
  words and returns `NF_ERROR_UNIMPLEMENTED` for an unrelated QuickDraw trap.

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

Phase 4.1 is complete. Proceed to Phase 4.2 rectangle operations.
