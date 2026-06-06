# Blocker: Phase 5 — missing real-module runner

## Summary

The maintainer supplied the local Flying Toasters module for the Phase 5
checkpoint, and the module was copied only into ignored local scratch with its
resource fork preserved. The repository still cannot perform the required
10-minute end-to-end checkpoint because the current tree has no real-module
execution path: no `tbtrace`, no `adinspect`, no app target, no Musashi wrapper,
and no `nf_module_load` / `nf_module_start` / `nf_advance` C ABI functions.

## Context

- Phase/subphase: Phase 5 — Memory/Resource/Sound-stub/misc Toolbox checkpoint
- Gate or invariant: Phase 5 requires a real-module 10-minute end-to-end run
  with zero unimplemented-trap warnings; phases must not be skipped or rebuilt;
  plan/reality contradictions require a blocker doc.
- First observed: 2026-06-06 after the maintainer supplied the local module.
- Command or check:
  - `find '/Users/james/Downloads/After Dark 4.94 Collection/After Dark Files/After Dark 2.0' -maxdepth 3 -type f -print`
  - `ditto <local module> .nightfall/manual-modules/Flying\ Toasters`
  - `git status --ignored --short .nightfall/manual-modules`
  - `rg -n "nf_module_load|nf_module_start|nf_advance|ModuleRuntime|tbtrace|adinspect|Musashi|m68k|ADgy|ADif|MacBinary|ResourceFile" . --glob '!build*/**' --glob '!.git/**'`
  - `nm -g build/libnightfall_core.a | rg "nf_module|nf_advance|nf_context|trap"`

## Evidence

- The local module exists at the maintainer-provided path.
- The local scratch copy is ignored by git:
  `!! .nightfall/`.
- The scratch copy preserves classic Mac metadata:
  resource fork size `30903` bytes, data fork size `0` bytes.
- `git ls-files` shows no `app/` files and no `tools/adinspect` or
  `tools/tbtrace` implementation in the current repository state.
- `core/nightfall.h` exposes only context, ticking, trap registration/dispatch,
  and fixture execution. It does not expose module loading/start/advance APIs.
- `nm -g build/libnightfall_core.a` shows `nf_context_*` and trap symbols, but
  no `nf_module_*` or `nf_advance` symbols.
- `rg` finds module-runtime, Musashi, ResourceFile, `tbtrace`, `adinspect`, and
  `nf_module_load` references only in planning documents, not implementation
  files.

## Requested Adjudication

Choose the smallest valid resume path:

Option 1: Provide or restore the missing prior-phase implementation artifacts
that the plan says already exist: module/resource loading, Musashi execution,
`tbtrace` or equivalent real-module trace runner, and the relevant C ABI surface.
Then resume Phase 5 checkpoint validation using the supplied local module from
ignored scratch.

Option 2: Authorize a scoped backfill of the missing real-module runner
artifacts before completing Phase 5, with explicit guidance that this is
reconciliation of absent Phase 0-3 deliverables rather than a Phase 5 scope
expansion.

Option 3: Adjudicate that the current reduced repository intentionally lacks the
real-module runtime, and authorize Phase 5 to proceed on synthetic evidence only
while carrying the first real-module end-to-end run to a later phase.

Recommended option: Option 1 if the artifacts exist in another branch or prior
state, because the plan says Phases 0-3 are complete and Phase 5 should not
rebuild them. If they do not exist, Option 2 is the clearest repair path.

## Resume

Resume only after the maintainer appends an adjudication block below with
`**Resume:** yes`.

## Adjudication 2026-06-06 — Option 2

Decision: Option 2. Authorize a scoped backfill of the missing real-module
runner/API artifacts needed to run the Phase 5 checkpoint. This is
reconciliation of absent prior-phase deliverables, not a Phase 5 scope change.

Rationale: The real-module runner is the thing Nightfall is building, but the
plan had treated its initial runner/API pieces as completed prior-phase
infrastructure. The current repository does not contain them, so a scoped
backfill is required before Phase 5 can honestly run the checkpoint.

**Resume:** yes
