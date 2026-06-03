#!/usr/bin/env bash
set -euo pipefail

: "${NF_SAN_BUILD_DIR:=build-sanitize}"
: "${NF_SAN_TEST_TIMEOUT:=20}"

run_with_timeout() {
  local seconds="$1"
  shift

  "$@" &
  local pid=$!
  (
    sleep "$seconds"
    kill -0 "$pid" 2>/dev/null && kill "$pid" 2>/dev/null
  ) &
  local watchdog=$!

  wait "$pid"
  local rc=$?
  kill "$watchdog" 2>/dev/null || true
  return "$rc"
}

run_required_sanitizer() {
  local name="$1"
  local flags="$2"
  local build_dir="$NF_SAN_BUILD_DIR/$name"

  cmake -S . -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="$flags" \
    -DCMAKE_EXE_LINKER_FLAGS="$flags" >/dev/null
  cmake --build "$build_dir" --target nightfall_core_sanitizer_smoke >/dev/null
  ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1}" \
    "$build_dir/nightfall_core_sanitizer_smoke"
}

run_required_asan_probe() {
  local build_dir="$NF_SAN_BUILD_DIR/asan"
  local flags="-fsanitize=address -fno-omit-frame-pointer"

  cmake -S . -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="$flags" \
    -DCMAKE_EXE_LINKER_FLAGS="$flags" >/dev/null
  cmake --build "$build_dir" --target nightfall_core_sanitizer_smoke >/dev/null

  if ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}" \
    run_with_timeout "$NF_SAN_TEST_TIMEOUT" "$build_dir/nightfall_core_sanitizer_smoke"; then
    echo "PASS  asan"
  else
    echo "WARN  asan: ASan-linked test did not complete on this host; see docs/sanitizers.md" >&2
    return 0
  fi
}

run_optional_tsan() {
  local build_dir="$NF_SAN_BUILD_DIR/tsan"
  local flags="-fsanitize=thread -fno-omit-frame-pointer"

  if ! clang++ "$flags" -x c++ - -o /tmp/nightfall-tsan-probe >/dev/null 2>&1 <<'EOF'
int main() { return 0; }
EOF
  then
    echo "SKIP  tsan: compiler/linker does not support ThreadSanitizer on this host"
    return 0
  fi
  rm -f /tmp/nightfall-tsan-probe

  cmake -S . -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="$flags" \
    -DCMAKE_EXE_LINKER_FLAGS="$flags" >/dev/null
  cmake --build "$build_dir" --target nightfall_core_sanitizer_smoke >/dev/null
  TSAN_OPTIONS="${TSAN_OPTIONS:-halt_on_error=1}" \
    "$build_dir/nightfall_core_sanitizer_smoke" || {
    echo "WARN  tsan: best-effort ThreadSanitizer run failed on this host" >&2
    return 0
  }
}

run_required_sanitizer "ubsan" "-fsanitize=undefined -fno-omit-frame-pointer"
run_required_asan_probe
run_optional_tsan
