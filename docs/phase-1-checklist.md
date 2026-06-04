# Phase 1 Completion Checklist

Phase 1 is complete when the repo proves the core bootstrap exists under the
clean-room and co-location rules.

## Requirements

- Deterministic core context exists behind the C ABI, using a fixed static
  context pool rather than heap allocation.
- Public source citations are recorded before trap or fixture execution behavior.
- A-line trap classification and dispatch scaffold exist.
- A bootstrap fixture execution path can dispatch A-line traps from assembled
  68k words and stop deterministically.
- Core implementation units have co-located tests, fixtures, and documentation.
- C++ tests use the locked doctest framework.
- Local build, tests, gates, assembler smoke, and gate meta-tests pass.

## Evidence

- C ABI and implementation: `core/nightfall.h`, `core/nightfall.cpp`.
- Tests: `core/nightfall.test.cpp`.
- Fixtures and docs: `core/nightfall.fixture.s`, `core/nightfall.md`,
  `core/fixtures/smoke.fixture.s`, `core/fixtures/smoke.md`.
- Citations: `docs/clean-room-sources.md`.
- Trace schema: `docs/trace-events.toml`.
- Test framework pin and vendored header: `third_party/VERSIONS.toml`,
  `third_party/doctest/doctest.h`.
- Verification command: `scripts/ci/run-local.sh`.
