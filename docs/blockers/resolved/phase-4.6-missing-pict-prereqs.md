# Blocker: Phase 4.6 — missing PICT generator and fuzz prerequisites

## Summary

Phase 4.6 cannot start under the plan as written because its mandatory PICT
coverage and fuzzing prerequisites are absent from the repository. The plan says
Phase 1 already delivered `tools/pict-gen/`, and Phase 4.6 requires PICT 2 opcode
coverage to match that generator plus mandatory `fuzz-pict`. The current tree has
no `tools/` directory and no `fuzz-pict` artifact.

This is a plan-state contradiction, not a failing implementation: the bootstrap
prompt says Phases 0-3 are complete and must not be rebuilt, while Phase 4.6
depends on prior-phase/tooling artifacts that are not present.

## Context

- Phase/subphase: Phase 4.6 / QuickDraw PICT interpreter
- Gate or invariant: sequential phase execution; do not rebuild or invent
  missing Phase 0-3/Phase 1 scaffolding; `fuzz-pict` mandatory for Phase 4.6
- First observed: 2026-06-04 after PR #13 merged and `docs/current-phase.md`
  advanced to Phase 4.6
- Command or check: `find tools core tests docs -maxdepth 3 -iname '*pict*' -o
  -iname '*PICT*'`, `rg -n "PICT|DrawPicture|pict-gen|fuzz-pict|Picture" . -g
  '!build*'`

## Evidence

- `docs/current-phase.md` requires:
  - PICT 2 opcode coverage matching `tools/pict-gen` coverage.
  - `fuzz-pict` mandatory for this subphase.
  - Picture decoding, pixel state, and parser error assertions.
- `nightfall-plan-macos.md` says Phase 1 delivers `tools/pict-gen/` initial
  fixture generation.
- `nightfall-plan-macos.md` says Phase 4.6 requires `DrawPicture` plus PICT 2
  opcodes where coverage equals `pict-gen` coverage, and `fuzz-pict` is
  mandatory.
- `find tools ...` reported: `find: tools: No such file or directory`.
- `rg -n "PICT|DrawPicture|pict-gen|fuzz-pict|Picture" . -g '!build*'` found
  only plan/tracker references, not implementation, fixture generator, or fuzzer
  artifacts.

## Requested Adjudication

Please choose the intended source of truth for Phase 4.6's PICT prerequisites.

Option 1: restore or provide maintainer-approved Phase 1 `tools/pict-gen/` and
`fuzz-pict` artifacts, then resume Phase 4.6 against those artifacts.

Option 2: explicitly authorize the agent to backfill the missing PICT generator
and `fuzz-pict` inside Phase 4.6, including the smallest acceptable opcode set
and fuzz scope. This changes the prior-phase boundary and should be recorded as a
decision.

Option 3: revise the Phase 4.6 gate so `DrawPicture` no longer depends on
`tools/pict-gen` or mandatory `fuzz-pict`. This weakens the validation contract
and is not recommended.

Agent recommendation: Option 1. The plan treats `pict-gen` and structure-aware
PICT fuzzing as already-established infrastructure. Restoring or supplying those
artifacts preserves the sequential phase contract and avoids inventing Phase 1
scope inside Phase 4.6.

## Resume

Resume only after the maintainer appends an adjudication block below with
`**Resume:** yes`.

## Adjudication 2026-06-04

Decision:

**Resume:** no

## Adjudication 2026-06-04 — Option 2

**Decision:** Option 2: authorize the agent to backfill Phase 1
`tools/pict-gen/` and the in-repo `fuzz-pict` libFuzzer target as Phase 4.6
prerequisites, with the smallest PICT 2 opcode set needed for Phase 4.6
validation.
**Rationale:** Required prior artifacts are absent from the repo; backfilling
them is accepted as prerequisite recovery for this phase so Phase 4.6 can
continue without weakening its validation contract.
**Decisions-log entry:** `docs/decisions-log.md`, "2026-06-04 — Phase 4.6
PICT prerequisite backfill".
**Resume:** yes
