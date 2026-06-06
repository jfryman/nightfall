# Phase 5 Validation

## Scope

Phase 5 implements the required Memory Manager, Resource Manager,
virtual-time/misc, and silent Sound Manager routines listed in
`docs/current-phase.md`.

Implemented routines:

- `NewHandle`, `DisposeHandle`, `GetHandleSize`, `SetHandleSize`, `HLock`,
  `HUnlock`, `HGetState`, `HSetState`
- `NewPtr`, `DisposePtr`
- `GetResource`, `ReleaseResource`, `GetResourceSizeOnDisk`, `DetachResource`
- `TickCount`, `GetDateTime`, `Delay`, `LMGetTicks`
- Silent `SndNewChannel`, `SndPlay`, `SndDoCommand`

## Evidence

- Clean-room citations recorded first in `docs/clean-room-sources.md` under
  "Phase 5 Toolbox Managers".
- Co-located implementation, tests, fixture, and documentation:
  `core/ToolboxManagers.cpp`, `core/ToolboxManagers.test.cpp`,
  `core/ToolboxManagers.fixture.s`, `core/ToolboxManagers.md`.
- Unit tests assert handle lifecycle, state flags, pointer disposal, resource
  loading/release/detach, deterministic virtual time, and silent sound command
  state.

## Local Gate Log

- PASS: `scripts/ci/run-local.sh`

## Checkpoint

The Phase 5 real-module checkpoint requires a maintainer-manual Flying Toasters
10-minute end-to-end run with zero unimplemented-trap warnings. That evidence is
not produced by CI and must be supplied as a sanitized committed artifact, or
this phase must open a blocker requesting it.

Result: maintainer supplied the local module, resolving the missing-input
blocker at
`docs/blockers/resolved/phase-5-missing-flying-toasters-artifact.md`.
The checkpoint remains blocked because the current repository lacks a
real-module execution runner/API:
`docs/blockers/resolved/phase-5-missing-real-module-runner.md`.

After adjudication, the minimal runner backfill was started:

- `core/ResourceFork.cpp` parses raw Macintosh resource fork bytes into a
  type/ID index using Inside Macintosh Resource Manager documentation.
- `core/M68KRuntime.cpp` wraps pinned Musashi execution with bounded memory,
  reset vectors, A-line trap capture, and synthetic tests.
- `tools/tbtrace` reads a local module's macOS resource fork xattr and runs the
  `ADgm/0` resource without committing module bytes.

Local real-module probe against ignored
`.nightfall/manual-modules/Flying Toasters`:

- resource fork bytes: 30903
- resource count: 27
- `ADgm` count: 1
- `ADgm/0` bytes: 12402
- default entry result: `status: ok`, `stop-reason: rts`, `trap-count: 0`,
  `unimplemented-trap-count: 0`

Result: the backfilled runner can parse and enter the real module, but the
remaining checkpoint work requires the After Dark graphics-module lifecycle ABI.
Work on that affected ABI is stopped by
`docs/blockers/phase-5-after-dark-lifecycle-contamination.md` after public web
search output exposed Berkeley Systems sample source/header material.
