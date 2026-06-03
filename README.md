# Nightfall

An After Dark–compatible screensaver for modern macOS that runs original 68k
modules — and, more to the point, a testbed for a repeatable agentic-development
pattern. *The screensaver is the excuse; the pattern is the product.*

This repository is set up to be executed by an autonomous agent (Codex) running
on the build host under an external supervisor, with the maintainer adjudicating
blockers asynchronously. The current run is scoped to the **Phases 0–3
checkpoint** (see `docs/decisions-log.md`).

## Read order

1. `nightfall-plan-macos.md` — the plan: scope, architecture, the 22 committed
   decisions (Q1–Q22), phases, gates. Canonical for *what* and *when to stop*.
2. `nightfall-agent-prompt-macos-v2.md` — the agent's bootstrap brief.
3. `docs/autonomous-operation.md` — the loop contract (how it runs unattended).
4. `docs/preflight.md` — what must be provisioned before launch.
5. `AGENTS.md` — the always-in-context summary the agent re-reads after compaction.

## Run it

```sh
./scripts/preflight.sh            # read-only; tells you what to authorize
#   ...fill VERSIONS.toml, register the runner, set a notify backend, etc...
./scripts/preflight.sh            # repeat until exit 0
./scripts/supervise.sh --dry-run  # preflight + show the plan, no launch
./scripts/supervise.sh            # bounded autonomous loop
touch docs/HALT                   # kill switch: stops cleanly after the cycle
```

Configuration knobs (env): `NF_MAX_HOURS`, `NF_MAX_ITERS`, `NF_MAX_STUCK`,
`NF_ITER_TIMEOUT_SEC`, `NF_CODEX_CMD`, `NF_NOTIFY_BACKEND` (+ the backend's secret).

## What's here vs. what the agent builds

Shipped (governance/bootstrap): the plan, the prompt, `AGENTS.md`, the `docs/`
contract + templates + decisions log, the `scripts/` (preflight / supervise /
notify), and `third_party/VERSIONS.toml` (pins to fill).

**Built by the agent during execution** (do not pre-create — that defeats the
spike): the enforcement gates (`nflint`, `tbcover`, `abi-guard`, `boundary-check`,
the PR-checklist bot), the CI workflow, `CMakeLists.txt`, the XcodeGen project,
the C ABI header `nightfall.h`, and everything under `core/`.

## Decisions and blockers

Settled decisions live in `docs/decisions-log.md`. When the agent hits something
the plan doesn't cover, it writes a blocker (`docs/blockers/`, template provided),
buzzes the maintainer, and halts; you unblock by appending an `## Adjudication …
**Resume:** yes` block to that doc.
