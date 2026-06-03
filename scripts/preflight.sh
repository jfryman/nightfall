#!/usr/bin/env bash
#
# Nightfall preflight gate.
#
# Read-only. Authorizes NOTHING. It probes every permission, credential,
# toolchain, and network door the autonomous loop needs, reports PASS / WARN /
# FAIL / SKIP with a one-line remediation, and exits non-zero if any hard
# requirement is unmet. Intended workflow:
#
#     ./scripts/preflight.sh        # see what's red
#     <you authorize the red items> # grant network/scopes/install tools
#     ./scripts/preflight.sh        # repeat until exit 0
#
# The supervisor runs this before launching Codex and will not launch on a
# non-zero exit. The agent re-runs it on every reconciliation (it is cheap and
# idempotent). It never mutates the repo or your environment.
#
# Scope: Phase A (build the app). Phase-B-only items (cloud signing, KMS) are reported
# SKIP, not FAIL; Phase A local signing is checked in section 10.
#
# Usage: ./scripts/preflight.sh [--test-slack] [--ci-mode actions|local]

# Deliberately NOT `set -e`: we want to run every check and summarise, not abort
# on the first failure.
set -uo pipefail

# ---------------------------------------------------------------------------
# Configuration (override via environment)
# ---------------------------------------------------------------------------
CI_MODE="${NF_CI_MODE:-actions}"        # "actions" (self-hosted runner, mechanical
                                        # merge gate) or "local" (agent runs gates
                                        # then merges via gh). Still an open fork;
                                        # default is the structurally-safer one.
PROTECTED_BRANCH="${NF_BRANCH:-main}"
NOTIFY_BACKEND="${NF_NOTIFY_BACKEND:-slack}"
MIN_MACOS_MAJOR="${NF_MIN_MACOS:-14}"
VERSIONS_FILE="${NF_VERSIONS_FILE:-third_party/VERSIONS.toml}"
TEST_NOTIFY=0

# Tools the build needs across Phase A (some not used until later phases).
#   name|hard|why
REQUIRED_TOOLS=(
  "git|1|version control"
  "gh|1|PR creation + self-merge-on-green"
  "codex|1|the worker agent"
  "cmake|1|builds libnightfall (C++ authority)"
  "xcodegen|1|generates Nightfall.xcodeproj"
  "clang|1|C++17 + libFuzzer (-fsanitize=fuzzer)"
  "vasm|1|assembles 68k .AD fixtures (Phase 1)"
  "jq|1|parsing gh api responses in CI scripts"
  "abidiff|0|abi-guard ABI gate (see note re: Mach-O below)"
)

# Repo doc/artefact preconditions.
REQUIRED_PATHS=(
  "AGENTS.md|1|Codex entry point (survives context compaction)"
  "docs/blockers/TEMPLATE.md|1|STOP-gate blocker template"
  "docs/autonomous-operation.md|1|loop contract"
  "docs/preflight.md|0|human provisioning checklist"
)

# Hosts the bootstrap must reach (vendoring + GitHub + Slack).
NET_HOSTS=(
  "github.com|1|git remote / gh"
  "api.github.com|1|gh api: branch protection, runners, PR merge"
  "codeload.github.com|1|fetching vendored deps (Musashi, glsl-crt)"
  "raw.githubusercontent.com|0|raw dependency manifests"
  "hooks.slack.com|0|heartbeat webhook"
)

# ---------------------------------------------------------------------------
# Output helpers
# ---------------------------------------------------------------------------
if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
  C_GREEN=$'\e[32m'; C_YELLOW=$'\e[33m'; C_RED=$'\e[31m'
  C_BLUE=$'\e[34m'; C_DIM=$'\e[2m'; C_BOLD=$'\e[1m'; C_OFF=$'\e[0m'
else
  C_GREEN=''; C_YELLOW=''; C_RED=''; C_BLUE=''; C_DIM=''; C_BOLD=''; C_OFF=''
fi

n_pass=0; n_warn=0; n_fail=0; n_skip=0
declare -a REMEDIATIONS=()

pass() { printf '  %sPASS%s  %s\n' "$C_GREEN" "$C_OFF" "$1"; n_pass=$((n_pass+1)); }
skip() { printf '  %sSKIP%s  %s\n' "$C_DIM"   "$C_OFF" "$1"; n_skip=$((n_skip+1)); }
warn() {
  printf '  %sWARN%s  %s\n' "$C_YELLOW" "$C_OFF" "$1"; n_warn=$((n_warn+1))
  [ $# -ge 2 ] && { printf '        %s→ %s%s\n' "$C_DIM" "$2" "$C_OFF"; REMEDIATIONS+=("WARN: $2"); }
}
fail() {
  printf '  %sFAIL%s  %s\n' "$C_RED" "$C_OFF" "$1"; n_fail=$((n_fail+1))
  [ $# -ge 2 ] && { printf '        %s→ %s%s\n' "$C_DIM" "$2" "$C_OFF"; REMEDIATIONS+=("FAIL: $2"); }
}
section() { printf '\n%s%s%s\n' "$C_BOLD$C_BLUE" "$1" "$C_OFF"; }
have() { command -v "$1" >/dev/null 2>&1; }

# Reachability without depending on curl flags being uniform across versions.
reachable() {
  local host="$1"
  if have curl; then
    curl -sS --max-time 6 -o /dev/null "https://${host}" 2>/dev/null && return 0
    # A TLS/HTTP error still proves the socket opened; only treat connect-level
    # failures as unreachable.
    local rc=$?
    [ "$rc" -ne 6 ] && [ "$rc" -ne 7 ] && [ "$rc" -ne 28 ] && return 0
    return 1
  elif have nc; then
    nc -z -G 6 "$host" 443 >/dev/null 2>&1
  else
    return 2  # cannot test
  fi
}

# ---------------------------------------------------------------------------
# Arg parsing
# ---------------------------------------------------------------------------
while [ $# -gt 0 ]; do
  case "$1" in
    --test-notify|--test-slack) TEST_NOTIFY=1 ;;
    --ci-mode) shift; CI_MODE="${1:-actions}" ;;
    -h|--help) grep '^#' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) printf 'unknown arg: %s\n' "$1" >&2; exit 2 ;;
  esac
  shift
done

printf '%sNightfall preflight%s  %s(ci-mode: %s, branch: %s)%s\n' \
  "$C_BOLD" "$C_OFF" "$C_DIM" "$CI_MODE" "$PROTECTED_BRANCH" "$C_OFF"

# ---------------------------------------------------------------------------
# 0. Kill switch — checked first; its presence is an intentional halt.
# ---------------------------------------------------------------------------
section "0. Kill switch"
if [ -f docs/HALT ]; then
  fail "docs/HALT present — loop is intentionally halted" \
       "remove docs/HALT to allow the loop to run"
else
  pass "no docs/HALT (loop not halted)"
fi

# ---------------------------------------------------------------------------
# 1. Build host
# ---------------------------------------------------------------------------
section "1. Build host"
if [ "$(uname -s)" = "Darwin" ]; then
  pass "running on macOS (Darwin)"
  if [ "$(uname -m)" = "arm64" ]; then
    pass "Apple Silicon (arm64)"
  else
    warn "not arm64 ($(uname -m)) — plan assumes Apple Silicon" "run on the arm64 build target"
  fi
  if have sw_vers; then
    macos_ver="$(sw_vers -productVersion 2>/dev/null)"
    macos_major="${macos_ver%%.*}"
    if [ "${macos_major:-0}" -ge "$MIN_MACOS_MAJOR" ] 2>/dev/null; then
      pass "macOS ${macos_ver} (>= ${MIN_MACOS_MAJOR} floor)"
    else
      fail "macOS ${macos_ver} below the ${MIN_MACOS_MAJOR}+ floor" "upgrade or change the deployment floor"
    fi
  fi
else
  fail "not macOS (uname=$(uname -s))" "run preflight on the macOS build target (smugglebook)"
fi

# ---------------------------------------------------------------------------
# 2. Toolchain
# ---------------------------------------------------------------------------
section "2. Toolchain"
for entry in "${REQUIRED_TOOLS[@]}"; do
  IFS='|' read -r tool hard why <<<"$entry"
  if have "$tool"; then
    pass "$tool present  ${C_DIM}($why)${C_OFF}"
  elif [ "$hard" = "1" ]; then
    fail "$tool missing  ($why)" "install $tool (e.g. brew install $tool)"
  else
    warn "$tool missing  ($why)" "install $tool when its phase is reached"
  fi
done

# Xcode toolchain + license (a real authorization gate).
if have xcodebuild; then
  if xcodebuild -version >/dev/null 2>&1; then
    pass "xcodebuild usable ($(xcodebuild -version 2>/dev/null | head -1))"
  else
    fail "xcodebuild present but not usable (likely unaccepted licence)" \
         "sudo xcodebuild -license accept"
  fi
  sel="$(xcode-select -p 2>/dev/null)"
  case "$sel" in
    *Xcode*) pass "active developer dir is a full Xcode ($sel)" ;;
    *) warn "active dir is CommandLineTools, not full Xcode ($sel)" \
            "sudo xcode-select -s /Applications/Xcode.app/Contents/Developer" ;;
  esac
else
  fail "xcodebuild missing" "install Xcode and run xcode-select"
fi

# libFuzzer availability (clang feature, not just clang presence).
if have clang; then
  fuzz_probe="$(mktemp -t nf_fuzz_XXXX).c"
  printf 'int LLVMFuzzerTestOneInput(const unsigned char*d,unsigned long s){return 0;}\n' >"$fuzz_probe"
  if clang -fsanitize=fuzzer -c "$fuzz_probe" -o /dev/null >/dev/null 2>&1; then
    pass "clang -fsanitize=fuzzer works"
  else
    warn "clang has no libFuzzer support" "use the Xcode/Homebrew clang that ships libFuzzer"
  fi
  rm -f "$fuzz_probe"
fi

# ---------------------------------------------------------------------------
# 3. Network egress (Codex sandbox permission)
# ---------------------------------------------------------------------------
section "3. Network egress  ${C_DIM}(Codex sandbox must allow this)${C_OFF}"
net_testable=1
have curl || have nc || { net_testable=0; warn "neither curl nor nc available" "install curl to verify egress"; }
if [ "$net_testable" = "1" ]; then
  for entry in "${NET_HOSTS[@]}"; do
    IFS='|' read -r host hard why <<<"$entry"
    if reachable "$host"; then
      pass "reach $host  ${C_DIM}($why)${C_OFF}"
    elif [ "$hard" = "1" ]; then
      fail "cannot reach $host  ($why)" "authorize network egress in the Codex sandbox / check firewall + DNS"
    else
      warn "cannot reach $host  ($why)" "authorize egress to $host if you want this feature"
    fi
  done
fi

# ---------------------------------------------------------------------------
# 4. GitHub access + merge rights (self-merge-on-green)
# ---------------------------------------------------------------------------
section "4. GitHub access"
repo_slug=""
if have gh; then
  if gh auth status >/dev/null 2>&1; then
    pass "gh authenticated"
    repo_slug="$(gh repo view --json nameWithOwner -q .nameWithOwner 2>/dev/null)"
    if [ -n "$repo_slug" ]; then
      pass "repo resolved: $repo_slug"
      perm="$(gh api "repos/$repo_slug" --jq '.permissions.push // false' 2>/dev/null)"
      admin="$(gh api "repos/$repo_slug" --jq '.permissions.admin // false' 2>/dev/null)"
      if [ "$perm" = "true" ]; then
        pass "token has push (required to self-merge)"
      else
        fail "token lacks push on $repo_slug" "grant the token repo write; gh auth refresh -s repo"
      fi
      [ "$admin" = "true" ] && pass "token has admin (can set branch protection)" \
        || warn "token lacks admin" "branch-protection checks below need admin to verify/set"
    else
      fail "not inside a GitHub repo / cannot resolve remote" "run inside the repo with a github remote set"
    fi
  else
    fail "gh not authenticated" "gh auth login  (scopes: repo, workflow)"
  fi
fi

# Push capability (read-only probe — lists remote refs, writes nothing).
if have git && git rev-parse --git-dir >/dev/null 2>&1; then
  if git ls-remote >/dev/null 2>&1; then
    pass "git remote reachable for fetch/push"
  else
    fail "git ls-remote failed" "configure the remote + credentials (gh auth setup-git)"
  fi
else
  warn "not a git working tree yet" "git init + add remote during bootstrap"
fi

# ---------------------------------------------------------------------------
# 5. Merge gate shape (depends on the open CI fork)
# ---------------------------------------------------------------------------
section "5. Merge gate  ${C_DIM}(ci-mode: $CI_MODE)${C_OFF}"
if [ "$CI_MODE" = "actions" ]; then
  if [ -n "$repo_slug" ] && have gh; then
    prot="$(gh api "repos/$repo_slug/branches/$PROTECTED_BRANCH/protection" 2>/dev/null)"
    if [ -n "$prot" ]; then
      checks="$(printf '%s' "$prot" | jq -r '.required_status_checks.contexts // [] | length' 2>/dev/null)"
      if [ "${checks:-0}" -gt 0 ] 2>/dev/null; then
        pass "$PROTECTED_BRANCH protected with $checks required check(s) — red merges are blocked mechanically"
      else
        fail "$PROTECTED_BRANCH protected but 0 required checks" \
             "add the gate set to required status checks (else self-merge-on-green is not enforced)"
      fi
    else
      warn "no branch protection on $PROTECTED_BRANCH yet" \
           "expected at bootstrap — the gate checks are a Phase 0 deliverable; lock protection + required checks once Phase 0 has built them (required for Phase 1+ merges)"
    fi
    runners="$(gh api "repos/$repo_slug/actions/runners" --jq '[.runners[]?|select(.status=="online")]|length' 2>/dev/null)"
    if [ "${runners:-0}" -gt 0 ] 2>/dev/null; then
      pass "$runners self-hosted runner(s) online"
    else
      warn "no online self-hosted runner" "register smugglebook as a self-hosted macOS runner so CI can execute"
    fi
  else
    warn "cannot verify branch protection (gh/repo unresolved)" "resolve section 4 first"
  fi
else
  # local mode: the gate scripts must exist and be runnable
  for s in nflint.py tbcover.py abi-guard.sh boundary-check.py; do
    if [ -x "scripts/ci/$s" ] || [ -f "scripts/ci/$s" ]; then
      pass "gate script present: scripts/ci/$s"
    else
      warn "gate script missing: scripts/ci/$s" "agent builds it in Phase 0; absent now is expected pre-bootstrap"
    fi
  done
  warn "local ci-mode: 'green' rests on the agent running gates before merge" \
       "prefer ci-mode=actions for a mechanical merge gate"
fi

# ---------------------------------------------------------------------------
# 6. Pinned versions
# ---------------------------------------------------------------------------
section "6. Dependency pinning"
if [ -f "$VERSIONS_FILE" ]; then
  if grep -Eq 'TODO|FIXME|XXX|<pin>|""[[:space:]]*$' "$VERSIONS_FILE"; then
    fail "$VERSIONS_FILE has unfilled pins" "fill exact commits/tags, or commit the default-policy marker"
  else
    pass "$VERSIONS_FILE present and has no unfilled-pin markers"
  fi
else
  fail "$VERSIONS_FILE missing" "create it with Musashi / nlohmann-json / glsl-crt / vasm / toolchain pins"
fi

# ---------------------------------------------------------------------------
# 7. Repo artefacts
# ---------------------------------------------------------------------------
section "7. Repo artefacts"
for entry in "${REQUIRED_PATHS[@]}"; do
  IFS='|' read -r p hard why <<<"$entry"
  if [ -e "$p" ]; then
    pass "$p  ${C_DIM}($why)${C_OFF}"
  elif [ "$hard" = "1" ]; then
    fail "$p missing  ($why)" "add it before launch"
  else
    warn "$p missing  ($why)" "add it before launch"
  fi
done

# ---------------------------------------------------------------------------
# 8. Heartbeat / notification
# ---------------------------------------------------------------------------
section "8. Notification (mobile-first)  ${C_DIM}(backend: $NOTIFY_BACKEND)${C_OFF}"
notify_secret=""
notify_var=""
case "$NOTIFY_BACKEND" in
  none)     skip "NF_NOTIFY_BACKEND=none — no push alerts (CI/dry runs only)" ;;
  slack)    notify_var="NIGHTFALL_SLACK_WEBHOOK"; notify_secret="${NIGHTFALL_SLACK_WEBHOOK:-}" ;;
  ntfy)     notify_var="NTFY_URL";       notify_secret="${NTFY_URL:-}" ;;
  pushover) notify_var="PUSHOVER_TOKEN+PUSHOVER_USER"
            [ -n "${PUSHOVER_TOKEN:-}" ] && [ -n "${PUSHOVER_USER:-}" ] && notify_secret="set" ;;
  webhook)  notify_var="NF_WEBHOOK_URL"; notify_secret="${NF_WEBHOOK_URL:-}" ;;
  *)        fail "unknown NF_NOTIFY_BACKEND='$NOTIFY_BACKEND'" "use slack|ntfy|pushover|webhook|none" ;;
esac
if [ "$NOTIFY_BACKEND" != "none" ]; then
  if [ -n "$notify_secret" ]; then
    pass "$notify_var configured for backend '$NOTIFY_BACKEND'"
    if [ "$TEST_NOTIFY" = "1" ] && [ -f scripts/notify.sh ]; then
      if bash scripts/notify.sh "preflight: mobile alert channel OK" info; then
        pass "test alert sent via scripts/notify.sh (check your phone)"
      else
        warn "scripts/notify.sh returned non-zero" "verify the backend credentials"
      fi
    fi
  else
    warn "$notify_var not set — the loop runs blind (no mobile alerts)" \
         "set $notify_var so blockers reach your phone"
  fi
fi

# ---------------------------------------------------------------------------
# 9. Availability (laptop must not sleep while you're away)
# ---------------------------------------------------------------------------
section "9. Availability"
have caffeinate && pass "caffeinate available (keep awake while the loop runs)" \
  || warn "caffeinate missing" "wrap the supervisor in caffeinate -dimsu"
if have pmset; then
  if pmset -g 2>/dev/null | grep -Eq '(^|[[:space:]])sleep[[:space:]]+0'; then
    pass "system sleep disabled"
  else
    warn "system may sleep on AC — a sleeping laptop is a silently stalled loop" \
         "sudo pmset -c sleep 0  (and keep a logged-in session for GUI app-smoke)"
  fi
fi

# ---------------------------------------------------------------------------
# 10. Phase A signing (needed at Phase 7/10; Phase B cloud-signing not yet)
# ---------------------------------------------------------------------------
section "10. Phase A signing"
if have security && security find-identity -v -p codesigning 2>/dev/null | grep -q 'Developer ID'; then
  pass "Developer ID signing identity present (local signing for Phase 7/10)"
else
  warn "no Developer ID identity — Phase A local signing (Phase 7/10) will need one" \
       "install a Developer ID Application cert into the login keychain before Phase 7"
fi
skip "KMS / OIDC / Terraform (Q19) — Phase B, not now"

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
section "Summary"
printf '  %s%d pass%s  %s%d warn%s  %s%d fail%s  %s%d skip%s\n' \
  "$C_GREEN" "$n_pass" "$C_OFF" "$C_YELLOW" "$n_warn" "$C_OFF" \
  "$C_RED" "$n_fail" "$C_OFF" "$C_DIM" "$n_skip" "$C_OFF"

if [ "$n_fail" -gt 0 ]; then
  printf '\n%sBlocked.%s Authorize the FAIL items above, then re-run. To authorize:\n' "$C_RED$C_BOLD" "$C_OFF"
  for r in "${REMEDIATIONS[@]}"; do [ "${r#FAIL: }" != "$r" ] && printf '  • %s\n' "${r#FAIL: }"; done
  exit 1
fi

if [ "$n_warn" -gt 0 ]; then
  printf '\n%sClear to launch%s, with warnings worth resolving before walking away.\n' "$C_YELLOW$C_BOLD" "$C_OFF"
else
  printf '\n%sClear to launch.%s\n' "$C_GREEN$C_BOLD" "$C_OFF"
fi
exit 0

# Note on abi-guard: libabigail's `abidiff` is ELF/DWARF-oriented; on macOS the C
# ABI (nightfall.h) is Mach-O. Verify Mach-O support on your toolchain, or have
# Phase 2 substitute a symbol-dump/header-snapshot diff. Flagged WARN above so it
# surfaces now rather than mid-Phase-2.
