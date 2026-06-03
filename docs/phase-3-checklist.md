# Phase 3 Completion Checklist

Phase 3 is complete when the first-agent checkpoint is reached and the spike is
explicitly marked complete.

## Requirements

- Phase 0 gate foundation is present and merged.
- Phase 1 deterministic core bootstrap is present and merged.
- Phase 2 ABI/gate hardening is present and merged.
- Preflight passes on the macOS build host with no hard failures.
- The local gate suite passes on the macOS build host.
- The blocker/resume protocol has been exercised and resolved blockers are
  preserved in `docs/blockers/resolved/`.
- The notification path is configured and testable without committing secrets.
- `docs/current-phase.md` contains the terminal marker
  `**Spike status:** complete`.
- No Phase 4 work has begun.

## Evidence

- Phase 2 merge PR: `https://github.com/jfryman/nightfall/pull/1`.
- Phase 2 merge commit: `a625e67a045289520807342b3ec762494357ad55`.
- Required GitHub check: `local-gates`, passed on PR #1.
- Preflight verification: `NIGHTFALL_SLACK_WEBHOOK="$(launchctl getenv NIGHTFALL_SLACK_WEBHOOK)" scripts/preflight.sh --test-notify`.
- Local verification: `scripts/ci/run-local.sh`.
- Resolved blockers:
  - `docs/blockers/resolved/phase-2-asan-macos-26.md`
  - `docs/blockers/resolved/phase-2-preflight-branch-protection.md`

## Notes

- ASan remains temporarily best-effort only on this macOS 26.5 host by the
  2026-06-03 decision in `docs/decisions-log.md`.
- Dependency exact pins are still recorded as a non-blocking warning under the
  accepted bootstrap policy.
- Phase 4 is intentionally not started in this spike.
