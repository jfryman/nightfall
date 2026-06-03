#!/usr/bin/env bash
#
# Nightfall notification helper — send-only, best-effort, mobile-first.
#
#   scripts/notify.sh "<message>" [severity]
#   severity: progress | info | high | urgent     (default: info)
#
# Both the supervisor and the agent (Codex) call this. It is the ONLY place a
# notification secret is read, and it is read from the environment — never passed
# on the command line, never embedded in the agent's prompt. Delivery failures
# never fail the caller: a missed buzz must not stop the loop.
#
# Backend is chosen by NF_NOTIFY_BACKEND (default: slack):
#   slack    NIGHTFALL_SLACK_WEBHOOK=<incoming-webhook-url>
#            or macOS Keychain service nightfall/slack-webhook, account $USER
#   ntfy     NTFY_URL=https://ntfy.sh/<topic> (or self-hosted)  [NTFY_TOKEN optional]
#   pushover PUSHOVER_TOKEN=<app>  PUSHOVER_USER=<user/group key>
#   webhook  NF_WEBHOOK_URL=<url>   (POSTs {"text":..,"severity":..})
#   none     no-op (useful for dry runs / CI)
#
# Severity maps to each provider's priority so blockers reach you loudly and
# routine progress stays silent. Bash 3.2-safe.

set -uo pipefail

msg="${1:-}"
sev="${2:-info}"
if [ -z "$msg" ]; then
  echo "usage: notify.sh <message> [progress|info|high|urgent]" >&2; exit 2
fi

backend="${NF_NOTIFY_BACKEND:-slack}"
[ "$backend" = "none" ] && exit 0

if ! command -v curl >/dev/null 2>&1; then
  echo "notify: curl unavailable; skipping ($sev) $msg" >&2; exit 0
fi

title="Nightfall"
text="[nightfall] $msg"
json_esc() { printf '%s' "$1" | sed 's/\\/\\\\/g; s/"/\\"/g'; }

slack_hook_from_keychain() {
  command -v security >/dev/null 2>&1 || return 1

  local service="${NF_SLACK_WEBHOOK_KEYCHAIN_SERVICE:-nightfall/slack-webhook}"
  local account="${NF_SLACK_WEBHOOK_KEYCHAIN_ACCOUNT:-${USER:-}}"
  if [ -n "$account" ]; then
    security find-generic-password -a "$account" -s "$service" -w 2>/dev/null
  else
    security find-generic-password -s "$service" -w 2>/dev/null
  fi
}

case "$backend" in
  slack)
    hook="${NIGHTFALL_SLACK_WEBHOOK:-}"
    if [ -z "$hook" ]; then hook="$(slack_hook_from_keychain || true)"; fi
    if [ -z "$hook" ]; then
      echo "notify(slack): NIGHTFALL_SLACK_WEBHOOK unset and Keychain item missing" >&2
      exit 0
    fi
    curl -sS --max-time 8 -X POST -H 'Content-type: application/json' \
      --data "{\"text\":\"$(json_esc "$text")\"}" "$hook" >/dev/null 2>&1 || true
    ;;

  ntfy)
    url="${NTFY_URL:-}"
    if [ -z "$url" ]; then echo "notify(ntfy): NTFY_URL unset" >&2; exit 0; fi
    case "$sev" in urgent) prio=5 ;; high) prio=4 ;; progress) prio=2 ;; *) prio=3 ;; esac
    if [ -n "${NTFY_TOKEN:-}" ]; then
      curl -sS --max-time 8 \
        -H "Authorization: Bearer $NTFY_TOKEN" \
        -H "Title: $title" -H "Priority: $prio" -H "Tags: robot" \
        -d "$msg" "$url" >/dev/null 2>&1 || true
    else
      curl -sS --max-time 8 \
        -H "Title: $title" -H "Priority: $prio" -H "Tags: robot" \
        -d "$msg" "$url" >/dev/null 2>&1 || true
    fi
    ;;

  pushover)
    tok="${PUSHOVER_TOKEN:-}"; usr="${PUSHOVER_USER:-}"
    if [ -z "$tok" ] || [ -z "$usr" ]; then
      echo "notify(pushover): PUSHOVER_TOKEN/PUSHOVER_USER unset" >&2; exit 0; fi
    case "$sev" in urgent) prio=2 ;; high) prio=1 ;; progress) prio=-1 ;; *) prio=0 ;; esac
    if [ "$prio" = "2" ]; then
      # Emergency priority must specify retry/expire or Pushover rejects it.
      curl -sS --max-time 8 \
        --data-urlencode "token=$tok" --data-urlencode "user=$usr" \
        --data-urlencode "title=$title" --data-urlencode "message=$msg" \
        --data-urlencode "priority=2" --data-urlencode "retry=120" --data-urlencode "expire=3600" \
        https://api.pushover.net/1/messages.json >/dev/null 2>&1 || true
    else
      curl -sS --max-time 8 \
        --data-urlencode "token=$tok" --data-urlencode "user=$usr" \
        --data-urlencode "title=$title" --data-urlencode "message=$msg" \
        --data-urlencode "priority=$prio" \
        https://api.pushover.net/1/messages.json >/dev/null 2>&1 || true
    fi
    ;;

  webhook)
    url="${NF_WEBHOOK_URL:-}"
    if [ -z "$url" ]; then echo "notify(webhook): NF_WEBHOOK_URL unset" >&2; exit 0; fi
    curl -sS --max-time 8 -X POST -H 'Content-type: application/json' \
      --data "{\"text\":\"$(json_esc "$text")\",\"severity\":\"$(json_esc "$sev")\"}" \
      "$url" >/dev/null 2>&1 || true
    ;;

  *)
    echo "notify: unknown NF_NOTIFY_BACKEND='$backend'" >&2; exit 0 ;;
esac
exit 0
