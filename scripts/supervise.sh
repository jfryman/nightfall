#!/usr/bin/env bash
#
# Nightfall supervisor.
#
# The belt-and-suspenders layer that lives OUTSIDE the agent. The agent's own
# stop rules (cycle budgets, HALT checks) are honest-effort; the failure mode
# this guards against is a flailing agent, which is the least likely to honor
# them. So the hard ceiling, the kill switch, crash-restart, and the
# loop-liveness heartbeat live here, where the agent cannot defeat them.
#
# Each loop iteration is one non-interactive Codex run. Codex does a chunk of
# work and exits; the supervisor classifies WHY it exited by inspecting repo
# state (not by trusting the exit code), then decides: relaunch (resume), or
# stop. The agent's idempotent-resume contract (autonomous-operation.md §1)
# makes relaunch safe.
#
# Communication split: the AGENT writes durable in-repo state (current-phase.md,
# blocker docs); the SUPERVISOR reads state transitions and relays them to Slack.
# The agent therefore never needs the webhook secret — it only needs git.
#
# Usage:
#   ./scripts/supervise.sh            # run the bounded loop
#   ./scripts/supervise.sh --dry-run  # preflight + show plan, do not launch Codex
#
# Compatible with macOS's system bash (3.2); avoids bash-4-only features.

set -uo pipefail   # NOT -e: the loop must survive a failed iteration.

# ---------------------------------------------------------------------------
# Configuration (override via environment)
# ---------------------------------------------------------------------------
# Codex invocation. MUST be a non-interactive / full-auto mode whose sandbox
# allows file writes, git, and network (Codex needs network to vendor deps and
# to drive gh). The exact flags differ by Codex version — VERIFY against yours.
# It receives the kickoff prompt as the final argument.
: "${NF_CODEX_CMD:=codex exec --full-auto}"
: "${NF_KICKOFF:=Begin the Nightfall spike per AGENTS.md and nightfall-agent-prompt-macos-v2.md. Reconcile state first (autonomous-operation.md S1), then proceed.}"

# Hard ceilings (the whole point of this file).
: "${NF_MAX_HOURS:=24}"          # wall-clock budget for the whole run
: "${NF_MAX_ITERS:=40}"          # max Codex relaunches
: "${NF_MAX_STUCK:=3}"           # consecutive no-progress/failed iterations -> stop
: "${NF_ITER_TIMEOUT_SEC:=3600}" # hard cap on a single Codex run

# Plumbing.
: "${NF_LOG_DIR:=.nightfall/logs}"
: "${NF_BLOCKERS_DIR:=docs/blockers}"
: "${NF_PHASE_FILE:=docs/current-phase.md}"
: "${NF_PREFLIGHT:=scripts/preflight.sh}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
NOTIFY="$SCRIPT_DIR/notify.sh"

# Make the notification config visible to the Codex child so the AGENT can also
# push in-the-moment mobile alerts via notify.sh. These are send-only, low-blast-
# radius secrets; the agent calls the helper and never sees them in its prompt.
export NF_NOTIFY_BACKEND="${NF_NOTIFY_BACKEND:-slack}"
export NIGHTFALL_SLACK_WEBHOOK="${NIGHTFALL_SLACK_WEBHOOK:-}"
export NTFY_URL="${NTFY_URL:-}" NTFY_TOKEN="${NTFY_TOKEN:-}"
export PUSHOVER_TOKEN="${PUSHOVER_TOKEN:-}" PUSHOVER_USER="${PUSHOVER_USER:-}"
export NF_WEBHOOK_URL="${NF_WEBHOOK_URL:-}"

DRY_RUN=0
[ "${1:-}" = "--dry-run" ] && DRY_RUN=1

# ---------------------------------------------------------------------------
# Output + heartbeat
# ---------------------------------------------------------------------------
mkdir -p "$NF_LOG_DIR"
RUN_LOG="$NF_LOG_DIR/supervisor-$(date +%Y%m%d-%H%M%S).log"

log() { printf '%s  %s\n' "$(date '+%H:%M:%S')" "$*" | tee -a "$RUN_LOG"; }

# Heartbeat: durable line in the log + a mobile push via the shared helper. This
# is the loop's liveness signal — the one thing the agent cannot emit about its
# own death. hb "<message>" [severity]
hb() {
  local sev="${2:-info}"
  log "HEARTBEAT [$sev] $1"
  [ -x "$NOTIFY" ] || [ -f "$NOTIFY" ] && bash "$NOTIFY" "$1" "$sev" || true
}

# ---------------------------------------------------------------------------
# State inspection — classification is purely repo-state-based
# ---------------------------------------------------------------------------
repo_head() { git rev-parse HEAD 2>/dev/null || echo none; }
phase_sig() { [ -f "$NF_PHASE_FILE" ] && cksum "$NF_PHASE_FILE" 2>/dev/null | awk '{print $1}' || echo 0; }

halt_present() { [ -f docs/HALT ]; }

# An unresolved blocker is any doc under blockers/ (not TEMPLATE, not resolved/)
# lacking the durable resume signal.
unresolved_blocker() {
  local f
  for f in "$NF_BLOCKERS_DIR"/*.md; do
    [ -e "$f" ] || continue
    case "$f" in */TEMPLATE.md) continue ;; esac
    if ! grep -qiE '^\s*\*\*Resume:\*\*\s*yes|^\s*Resume:\s*yes' "$f" 2>/dev/null; then
      BLOCKER_PATH="$f"
      return 0
    fi
  done
  return 1
}

# Terminal state for the spike: the agent records this in current-phase.md when
# it reaches the chosen boundary (Phases 0-3 checkpoint).
boundary_reached() {
  [ -f "$NF_PHASE_FILE" ] && grep -qiE '^\s*\*\*Spike status:\*\*\s*complete' "$NF_PHASE_FILE" 2>/dev/null
}

# ---------------------------------------------------------------------------
# One Codex run, with an out-of-process watchdog (portable; no GNU timeout)
# ---------------------------------------------------------------------------
run_codex() {
  local iter="$1" out="$NF_LOG_DIR/codex-iter-$1.log"
  log "launching Codex (iteration $iter): $NF_CODEX_CMD"
  # shellcheck disable=SC2086
  $NF_CODEX_CMD "$NF_KICKOFF" >"$out" 2>&1 &
  local pid=$!
  # Watchdog: kill the run if it blows the per-iteration cap.
  ( sleep "$NF_ITER_TIMEOUT_SEC"; kill -0 "$pid" 2>/dev/null && {
      log "WARN: iteration $iter exceeded ${NF_ITER_TIMEOUT_SEC}s — terminating"; kill "$pid" 2>/dev/null; } ) &
  local wd=$!
  wait "$pid"; local rc=$?
  kill "$wd" 2>/dev/null
  return $rc
}

# ---------------------------------------------------------------------------
# Preflight gate
# ---------------------------------------------------------------------------
if [ ! -x "$NF_PREFLIGHT" ] && [ ! -f "$NF_PREFLIGHT" ]; then
  log "FATAL: $NF_PREFLIGHT not found"; exit 2
fi
log "running preflight gate"
if ! bash "$NF_PREFLIGHT"; then
  hb "preflight FAILED — not launching. Authorize the red items and re-run." high
  exit 1
fi

if [ "$DRY_RUN" = "1" ]; then
  log "dry-run: preflight passed. Would loop up to $NF_MAX_ITERS iterations / ${NF_MAX_HOURS}h."
  log "dry-run: Codex command would be: $NF_CODEX_CMD \"<kickoff>\""
  exit 0
fi

# ---------------------------------------------------------------------------
# Main bounded loop
# ---------------------------------------------------------------------------
START_TS=$(date +%s)
MAX_SECS=$(( NF_MAX_HOURS * 3600 ))
iter=0
stuck=0
hb "spike supervisor started (budget: ${NF_MAX_HOURS}h / ${NF_MAX_ITERS} iters)" info

while : ; do
  # --- pre-flight checks for this iteration --------------------------------
  if halt_present; then
    hb "HALTED — docs/HALT present. Stopping cleanly." high; exit 0
  fi
  now=$(date +%s); elapsed=$(( now - START_TS ))
  if [ "$elapsed" -ge "$MAX_SECS" ]; then
    hb "BUDGET-EXHAUSTED — wall-clock ceiling (${NF_MAX_HOURS}h) reached. Stopping." high; exit 0
  fi
  if [ "$iter" -ge "$NF_MAX_ITERS" ]; then
    hb "BUDGET-EXHAUSTED — iteration ceiling (${NF_MAX_ITERS}) reached. Stopping." high; exit 0
  fi

  # Don't relaunch into an open blocker — that would just re-hit the STOP.
  if unresolved_blocker; then
    hb "BLOCKED — awaiting adjudication: $BLOCKER_PATH . Append an '## Adjudication … **Resume:** yes' block and re-run the supervisor." urgent
    exit 0
  fi
  if boundary_reached; then
    hb "SPIKE COMPLETE — Phases 0-3 checkpoint reached. Stopping (success)." high; exit 0
  fi

  # --- run one iteration ---------------------------------------------------
  iter=$(( iter + 1 ))
  pre_head=$(repo_head); pre_phase=$(phase_sig)
  run_codex "$iter"; codex_rc=$?

  # --- classify the exit by state, not by exit code ------------------------
  if halt_present; then
    hb "HALTED mid-iteration. Stopping cleanly." high; exit 0
  fi
  if unresolved_blocker; then
    hb "BLOCKED after iteration $iter: $BLOCKER_PATH . Re-run after adjudicating." urgent; exit 0
  fi
  if boundary_reached; then
    hb "SPIKE COMPLETE after iteration $iter — Phases 0-3 reached. Stopping (success)." high; exit 0
  fi

  post_head=$(repo_head); post_phase=$(phase_sig)
  if [ "$post_head" != "$pre_head" ] || [ "$post_phase" != "$pre_phase" ]; then
    progress=1
  else
    progress=0
  fi

  if [ "$codex_rc" -ne 0 ] || [ "$progress" -eq 0 ]; then
    stuck=$(( stuck + 1 ))
    log "no-progress/failed iteration ($iter): rc=$codex_rc progress=$progress stuck=$stuck/$NF_MAX_STUCK"
    if [ "$stuck" -ge "$NF_MAX_STUCK" ]; then
      hb "STUCK — $stuck consecutive iterations with no forward progress and no blocker doc. Stopping for inspection (log: $RUN_LOG)." high
      exit 1
    fi
  else
    stuck=0
    hb "progress: iteration $iter advanced state (HEAD/phase changed). Continuing." progress
  fi
done
