# Phase 2 Completion Checklist

Phase 2 is complete when ABI and gate hardening are proven locally.

## Requirements

- C ABI boundary is protected by a Mach-O-compatible guard.
- ABI guard has known-good and known-bad meta-tests.
- Trace, boundary, coverage, dependency, tool, lint, and assembler gates remain
  wired into local verification.
- Required sanitizer profile passes: ASan and UBSan.
- Best-effort TSan is attempted or skipped with a clear host/toolchain reason.
- Local verification command reports green without unresolved required warnings.

## Current Evidence

- ABI guard: `scripts/ci/abi-guard.sh`, `docs/abi/nightfall.h.snapshot`,
  `docs/abi/nightfall.symbols`.
- ABI guard meta-tests: `tests/gates/abi-guard/`.
- Sanitizer runner: `scripts/ci/sanitizers.sh`.
- Sanitizer smoke harness: `core/nightfall.sanitizer.cpp`.
- Sanitizer status: `docs/sanitizers.md`.
- Local verification command: `scripts/ci/run-local.sh`.

## Open

- ASan is not yet passing on this macOS 26.5 host. Allocator-using ASan binaries
  time out during sanitizer runtime initialization before Nightfall code runs.
  Minimal probes show the same issue for allocator, global, and stack
  instrumentation. Mozilla Bug 2037587 corroborates this as a macOS 26 ASan
  runtime/toolchain deadlock.
