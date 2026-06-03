# Post-Spike Blocker: Scope Boundary Reached Before Phase 4

## Summary

The Phases 0-3 spike is complete and `docs/current-phase.md` contains the
terminal marker `**Spike status:** complete`. The maintainer now wants the agent
to continue through the remaining phases, but the committed scope documents
explicitly say this run stops at the first-agent checkpoint and does not begin
Phase 4.

## Trigger

- Gate, contradiction, or provisioning issue: plan scope contradiction.
- Why it cannot be worked around safely: `AGENTS.md`, `nightfall-plan-macos.md`,
  `docs/autonomous-operation.md`, and `docs/decisions-log.md` all record the
  Phases 0-3 checkpoint as a terminal run boundary. Beginning Phase 4 without a
  durable re-scope would violate the sequential plan and make future invocations
  disagree about whether Phase 4 work was authorized.

## Options

1. Recommended option: create a new post-spike scope decision that authorizes
   continuing beyond Phase 3, defines the next phase boundary, and updates the
   plan/current-phase docs before implementation begins.
2. Alternative: keep the Phases 0-3 spike closed and stop here; start a separate
   future thread/run for Phase 4 with a fresh AGENTS/current-phase scope.
3. Alternative: authorize a bounded planning-only PR that reconstructs/extends
   the missing canonical plan for Phase 4+ without starting product code.

## Recommendation

Use option 1 if the intent is to keep this agent driving the product tonight.
The adjudication should explicitly state the new boundary, for example "continue
from Phase 4 through Phase A, in phase chunks, with merged green PRs, stopping on
blockers, budget exhaustion, or finished product." Once recorded, the agent can
update `docs/decisions-log.md`, transition `docs/current-phase.md` out of the
spike-complete terminal state, and begin the next phase.

## Evidence

- Relevant command or check: reconciliation on `main` after PR #3.
- Relevant file or log:
  - `AGENTS.md`: "Phases 0 -> the first-agent checkpoint (Phases 0-3),
    terminal" and "Do not begin Phase 4."
  - `nightfall-plan-macos.md`: "Stop at the first-agent checkpoint after Phases
    0-3" and "Do not begin Phase 4."
  - `docs/autonomous-operation.md`: "Decision: run to the Phases 0-3 checkpoint,
    then stop" and "the agent does not begin Phase 4."
  - `docs/current-phase.md`: `**Spike status:** complete`.
  - `scripts/preflight.sh`: passes with warnings only.

## Adjudication

**Decision:** pending
**Rationale:** pending
**Decisions-log entry:** pending
**Resume:** no
