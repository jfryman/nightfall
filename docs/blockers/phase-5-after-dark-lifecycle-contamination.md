# Blocker: phase-5-after-dark-lifecycle-contamination

## Summary

Phase 5 cannot continue into After Dark graphics-module lifecycle entry
implementation because a public web search exposed Berkeley Systems sample
source/header material while investigating the `ADgm` entry convention.

## Context

- Phase/subphase: Phase 5, real-module runner backfill.
- Gate or invariant: Clean-room discipline; forbidden-source exposure requires
  stopping the affected subsystem and opening a blocker.
- First observed: 2026-06-06.
- Command or check: Web search for public After Dark `ADgm` module-format /
  lifecycle information after `tbtrace` loaded the local module but the default
  `ADgm/0` entry returned immediately without traps.

## Evidence

- The local runner successfully parsed the ignored maintainer-supplied module
  resource fork and loaded `ADgm/0`:
  - resource fork bytes: 30903
  - resource count: 27
  - `ADgm/0` bytes: 12402
  - default entry result: `status: ok`, `stop-reason: rts`, `trap-count: 0`
- Offset probing likewise did not produce Toolbox traps, which means the
  runner now needs the After Dark graphics-module lifecycle invocation ABI
  rather than another Resource Manager parser fix.
- A web search then surfaced pages under
  `https://smfr.org/computing/archaic/afterdark/adprog/` containing Berkeley
  Systems After Dark sample source/header material. The agent saw enough to
  treat the affected lifecycle ABI work as provenance-risky.
- No lifecycle ABI/message implementation was written from that material.

## Requested Adjudication

Please choose the smallest clean-room path for the After Dark lifecycle ABI:

1. Explicitly approve the public After Dark module programming pages as an
   allowable specification source for this project, despite the Berkeley sample
   source/header exposure.
2. Provide a maintainer-authored sanitized ABI artifact covering only the needed
   call convention, message values, parameter block layout, and entry sequence,
   with provenance acceptable to the project.
3. Direct rollback or quarantine of additional files beyond the affected
   lifecycle ABI work.

Recommended option: 2. It preserves the parser/runner backfill while keeping the
agent from deriving host ABI behavior from exposed Berkeley sample source.

## Resume

Resume only after the maintainer appends an adjudication block below with
`**Resume:** yes`.

## Adjudication 2026-06-06

Decision:

**Resume:** no
