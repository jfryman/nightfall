# Phase 4 Validation — QuickDraw Subset Checkpoint

## Scope

This checkpoint closes Phase 4 after the atomic QuickDraw subphases:

- Phase 4.1: init and port state
- Phase 4.2: rectangle drawing and color traps
- Phase 4.3: lines and pen state
- Phase 4.4: regions and clipping
- Phase 4.5: bit and pixel map transfer
- Phase 4.6: PICT interpreter and `DrawPicture`

## Q7 Decision Table

The original Q7 checkpoint requires color-depth validation against the seven
benchmark modules. Those maintainer-manual sanitized artifacts are not present in
the repository, and real modules are not loaded in CI.

The blocker at
`docs/blockers/resolved/phase-4-checkpoint-missing-q7-artifacts.md` was
adjudicated on 2026-06-06 with Option 2: proceed with a reduced synthetic-only
checkpoint using committed fixtures and generated PICT coverage.

Decision-table result for this checkpoint:

- **Proceed to Phase 5.**
- Do **not** insert the 8-bit indexed fallback now.
- Do **not** claim real-module Q7 benchmark validation from this checkpoint.
- Carry the missing real-module benchmark evidence forward as maintainer-manual
  evidence for later validation/release decisions.

## Evidence

- Phase 4.1 validation: `docs/phase-4.1-validation.md`
- Phase 4.2 validation: `docs/phase-4.2-validation.md`
- Phase 4.3 validation: `docs/phase-4.3-validation.md`
- Phase 4.4 validation: `docs/phase-4.4-validation.md`
- Phase 4.5 validation: `docs/phase-4.5-validation.md`
- Phase 4.6 validation: `docs/phase-4.6-validation.md`
- Toolbox coverage snapshot: `docs/toolbox-coverage.md`
- Synthetic fixture generation: `tools/pict-gen/`
- PICT fuzzing: `fuzz-pict`
- Local validation: `scripts/ci/run-local.sh`

## Local Validation

- PASS: `scripts/ci/run-local.sh`
