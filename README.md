# nightfall

Bootstrap repository for the Nightfall spike.

Imported from `/Users/james/Downloads/files.zip`:

- `AGENTS.md`
- `docs/autonomous-operation.md`
- `docs/preflight.md`
- `docs/decisions-log.md`
- `docs/attestations/TEMPLATE.md`
- `scripts/preflight.sh`
- `scripts/supervise.sh`
- `scripts/notify.sh`

Bootstrap scaffolding now also includes:

- a reconstructed [nightfall-plan-macos.md](/Users/james/repo/src/github.com/jfryman/nightfall/nightfall-plan-macos.md)
- a reconstructed [nightfall-agent-prompt-macos-v2.md](/Users/james/repo/src/github.com/jfryman/nightfall/nightfall-agent-prompt-macos-v2.md)
- a bootstrap [third_party/VERSIONS.toml](/Users/james/repo/src/github.com/jfryman/nightfall/third_party/VERSIONS.toml)
- placeholder clean-room and blocker workflow documents under
  [docs](/Users/james/repo/src/github.com/jfryman/nightfall/docs)
- a minimal CMake-based `nightfall_core` library and local test executable
- first-pass gate scripts for the core/app boundary and source linting

These files are intentionally explicit about what is reconstructed, what is
policy, and what still needs maintainer-provided exact values.

## Local Development

Run the current local build, tests, and early gate checks with:

```sh
scripts/ci/run-local.sh
```

This is the manual-session path. The autonomous supervisor remains available,
but it is not required for day-to-day bring-up.

Current local checks:

- CMake configure/build
- CTest
- required ASan/UBSan sanitizer run, with TSan best-effort
- `abi-guard`
- `boundary-check`
- `nflint`
- `depcheck`
- `toolcheck`
- `trace-schema-guard`
- `tbcover`
- 68k assembler fixture smoke
- gate meta-tests for each implemented gate
