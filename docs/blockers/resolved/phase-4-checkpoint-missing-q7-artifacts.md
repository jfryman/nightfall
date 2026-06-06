# Blocker: Phase 4 checkpoint — missing Q7 benchmark artifacts

## Summary

The Phase 4 checkpoint cannot be completed because the repository does not
contain maintainer-provided sanitized validation artifacts for the Q7
color-depth decision table.

The plan requires Q7 color-depth validation against the seven benchmark modules
after Phase 4.6. The bootstrap contract also says real modules are
maintainer-manual and must never be loaded in CI. Without committed sanitized
artifacts, the agent cannot prove the checkpoint, cannot choose the Q7 decision
table outcome, and must not invent substitute evidence.

## Context

- Phase/subphase: Phase 4 checkpoint after Phase 4.6
- Gate or invariant: Q7 color-depth validation; no real modules in CI; blocker
  when maintainer-manual artifacts are absent
- First observed: 2026-06-05 after PR #15 merged and `docs/current-phase.md`
  advanced to the Phase 4 checkpoint
- Command or check:
  - `find docs/analysis docs -maxdepth 3 -type f -print | sort`
  - `rg -n "Q7|color-depth|color depth|seven|benchmark|Flying|Fish|Lawnmower|Bad Dog|Starry|Warp|Rat Race|Phase 4 checkpoint|maintainer-manual|manual" docs nightfall-plan-macos.md AGENTS.md`

## Evidence

- `nightfall-plan-macos.md` states: "Phase 4 checkpoint: Q7 color-depth
  validation against all seven modules. Decision table -> proceed / insert Phase
  4.5+ (8-bit indexed, ~1 week) / STOP with blocker doc."
- `AGENTS.md` states that real-module validation is maintainer-manual and that
  absent artifacts are blockers; real modules must never be loaded in CI.
- `docs/current-phase.md` requires:
  - validation at `docs/phase-4-validation.md`;
  - Q7 decision-table result;
  - blocker doc if maintainer-manual artifacts are absent.
- `find docs/analysis ...` reported `find: docs/analysis: No such file or
  directory`.
- The tree contains Phase 4.1 through Phase 4.6 validation logs, but no
  `docs/phase-4-validation.md` and no sanitized benchmark-module validation
  artifacts.

## Requested Adjudication

Please provide the intended Phase 4 checkpoint evidence or revise the checkpoint
contract.

Option 1: provide committed sanitized maintainer-manual artifacts for the Q7
benchmark validation, then resume the checkpoint and apply the decision table.

Option 2: explicitly authorize a reduced synthetic-only Phase 4 checkpoint using
the committed fixtures and generated PICT coverage. This would weaken the Q7
real-module validation contract and should be recorded as a decision.

Option 3: revise the plan so the Q7 real-module color-depth checkpoint moves to
a later maintainer-manual phase, then resume from Phase 5.

Agent recommendation: Option 1. The checkpoint exists specifically to validate
the 32-bit direct-color bet against benchmark module behavior. Committed
sanitized maintainer artifacts preserve that evidence without loading real
modules in CI.

## Resume

Resume only after the maintainer appends an adjudication block below with
`**Resume:** yes`.

## Adjudication 2026-06-05

Decision:

**Resume:** no

## Adjudication 2026-06-06 — Option 2

**Decision:** Option 2: authorize a reduced synthetic-only Phase 4 checkpoint
using the committed fixtures, generated PICT coverage, and local gate suite.
Real-module Q7 benchmark evidence remains maintainer-manual and must be supplied
or revisited before any public-release claim that depends on those modules.
**Rationale:** The Phase 4 QuickDraw subset is complete and locally gated, but
the real-module artifacts are absent. Proceeding with an explicit synthetic-only
checkpoint keeps Phase A moving without pretending that the missing real-module
evidence exists.
**Decisions-log entry:** `docs/decisions-log.md`, "2026-06-06 — Phase 4
checkpoint synthetic-only resume".
**Resume:** yes
