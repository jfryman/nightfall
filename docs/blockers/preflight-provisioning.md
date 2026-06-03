# Blocker: preflight provisioning

## Summary

`scripts/preflight.sh` exited non-zero on 2026-06-03 before Nightfall work could
begin. Per the launch contract, normal project work is halted until the failing
preflight items are authorized and fixed.

## Preflight failures

The script reported these FAIL items:

- `vasm` missing: assembles 68k `.AD` fixtures (Phase 1).
  - Script guidance: install `vasm`, for example `brew install vasm`.
- `third_party/VERSIONS.toml` has unfilled pins.
  - Script guidance: fill exact commits/tags, or commit the default-policy marker.
- `docs/blockers/TEMPLATE.md` missing: STOP-gate blocker template.
  - Script guidance: add it before launch.

## Related warnings

The script also reported these WARN items:

- `abidiff` missing: install when its phase is reached.
- `NIGHTFALL_SLACK_WEBHOOK` is not set: mobile alerts will not be delivered.
- System may sleep on AC: set `sudo pmset -c sleep 0` and keep a logged-in
  session for GUI app-smoke.
- No Developer ID identity: Phase A local signing in Phase 7/10 will need one.

## Recommended resolution

Install `vasm`, complete or explicitly mark the dependency pins according to the
project policy, and restore `docs/blockers/TEMPLATE.md`. Then rerun
`scripts/preflight.sh`.

## Resume

Resume only after an adjudication block is appended below with `**Resume:** yes`.
