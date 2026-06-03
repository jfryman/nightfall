# Nightfall Plan for macOS

This document is a repo-local reconstruction of the missing canonical plan
referenced by `AGENTS.md`. It is derived from:

- `AGENTS.md`
- `docs/autonomous-operation.md`
- `docs/preflight.md`
- `docs/decisions-log.md`

If the original plan document is recovered later, compare and replace this file
deliberately rather than merging blindly.

## Objective

Nightfall is a macOS-only, After Dark-compatible screensaver host for original
68k modules. The spike goal is narrower than shipping the product: prove that a
gated, recoverable, largely unattended agent workflow can move the project from
bootstrap through the first checkpoint safely.

## Scope of this spike

- Start at Phase 0.
- Stop at the first-agent checkpoint after Phases 0-3.
- Do not begin Phase 4.
- Treat Phase 0.5 as non-blocking and keep "Nightfall" as a placeholder name.

Terminal condition:

`docs/current-phase.md` must contain exactly `**Spike status:** complete`.

## Hard constraints

- macOS only.
- Apple Silicon build and CI target: `smugglebook`.
- CMake is the build authority for `libnightfall`.
- XcodeGen produces the Xcode project.
- The C ABI seam exists for testability, not portability.
- `core/` remains pure C++17 plus the C ABI header, with no AppKit,
  Foundation, or Metal imports.
- Trace via structured `nf_trace_event`, never `printf` in `core/`.
- No dependency, architecture, or process invention outside recorded decisions.
- Clean-room discipline is mandatory and citation-first.

## Working architecture

- `core/`: deterministic emulation and host-independent logic.
- App / integration layer: macOS-specific boundary, rendering, packaging, and
  app-smoke validation.
- `scripts/`: bootstrap, supervision, notification, and gate helpers.
- `docs/`: current phase, blockers, clean-room provenance, and decisions.
- `third_party/`: dependency source-of-truth and vendored inputs as phases land.

## Phase outline

### Phase 0: Bootstrap and gate foundation

- Establish repo structure, bootstrap docs, and state tracking.
- Wire preflight, supervisor, notification, and initial CI/gate scaffolding.
- Land gate-meta-tests as an exit requirement for the phase, not a follow-up.

Exit focus:

- repo and process scaffolding are committed
- enforcement gates exist or are clearly staged
- gate-meta-test expectation is part of the contract

### Phase 0.5: Naming and legal placeholder

- Keep the project name as "Nightfall".
- Do not block engineering progress on trademark or legal review.

### Phase 1: Core bootstrap

- Begin deterministic core setup under the clean-room rules.
- Use public specifications cited in `docs/clean-room-sources.md` before any
  trap or host routine implementation.
- Maintain file co-location for implementation, tests, fixtures, and citations.

### Phase 2: ABI and gate hardening

- Harden the C ABI boundary.
- Validate ABI guard strategy on Mach-O or raise a blocker if uncertain.
- Continue building tested enforcement around trace, boundary, and coverage.

### Phase 3: First-agent checkpoint

- Reach the first checkpoint with the bootstrap loop, gates, trace pipeline,
  clean-room paper trail, and blocker/resume protocol all exercised.
- Mark the spike complete and stop.

## Gates

Required merge gate shape is `actions` mode with branch protection and required
checks. The intended gate set is:

- build
- tests
- sanitizer matrix
- `nflint`
- `tbcover`
- `abi-guard`
- `trace-schema-guard`
- `depcheck`
- `toolcheck`
- `boundary-check`
- `golden-frame-diff`
- `app-smoke` where applicable
- PR checklist bot

Every gate needs committed known-good and known-bad fixtures.

## Tooling decisions already locked

- C++ tests: doctest
- Fuzzing: libFuzzer
- Sanitizers: ASan and UBSan required, TSan best-effort for this spike
- Pinning policy: latest stable as of bootstrap date, exact hash recorded in
  `third_party/VERSIONS.toml`, `DEP-UPDATE:` required to change

## Blockers and resumption

- Missing provisioning, unresolved contradictions, timebox overruns, or unclear
  ABI strategy become blocker docs.
- Resume only after an adjudication block with `**Resume:** yes`.
- Do not continue parallel work while a blocker is open.

## Open risks carried forward

- The original canonical plan is not available in-tree; this reconstruction is
  a faithful synthesis, not a verbatim recovery.
- Mach-O-compatible ABI guard strategy is still open.
- Exact dependency pins and vendoring remain to be materialized once network and
  tool provisioning are available.
