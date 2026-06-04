#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

pass() { printf 'PASS %s\n' "$1"; }
fail() { printf 'FAIL %s\n' "$1" >&2; exit 1; }

expect_success() {
  local name="$1"
  shift
  "$@" >/tmp/nightfall-ci.out 2>/tmp/nightfall-ci.err || {
    cat /tmp/nightfall-ci.out
    cat /tmp/nightfall-ci.err >&2
    fail "$name"
  }
  pass "$name"
}

expect_failure() {
  local name="$1"
  shift
  if "$@" >/tmp/nightfall-ci.out 2>/tmp/nightfall-ci.err; then
    cat /tmp/nightfall-ci.out
    cat /tmp/nightfall-ci.err >&2
    fail "$name unexpectedly passed"
  fi
  pass "$name"
}

require_tool() {
  command -v "$1" >/dev/null 2>&1 || fail "missing required tool: $1"
}

doctest_include_dir() {
  if [ -f third_party/doctest/doctest.h ]; then
    printf '%s\n' "$ROOT_DIR/third_party/doctest"
    return
  fi
  if [ -f /opt/homebrew/include/doctest/doctest.h ]; then
    printf '%s\n' /opt/homebrew/include/doctest
    return
  fi
  if [ -f /usr/local/include/doctest/doctest.h ]; then
    printf '%s\n' /usr/local/include/doctest
    return
  fi
  fail "doctest.h not found"
}

run_build_and_tests() {
  local include_dir
  include_dir="$(doctest_include_dir)"
  rm -rf build
  cmake -S . -B build -DNIGHTFALL_DOCTEST_INCLUDE_DIR="$include_dir"
  cmake --build build
  ctest --test-dir build --output-on-failure
}

run_sanitizers() {
  local include_dir
  include_dir="$(doctest_include_dir)"
  local sanitizer_cxx="${NIGHTFALL_SANITIZER_CXX:-}"
  if [ -z "$sanitizer_cxx" ] && [ -x /opt/homebrew/opt/llvm/bin/clang++ ]; then
    sanitizer_cxx=/opt/homebrew/opt/llvm/bin/clang++
  fi
  if [ -z "$sanitizer_cxx" ]; then
    sanitizer_cxx="$(command -v clang++)"
  fi
  rm -rf build-sanitize
  cmake -S . -B build-sanitize \
    -DNIGHTFALL_DOCTEST_INCLUDE_DIR="$include_dir" \
    -DCMAKE_CXX_COMPILER="$sanitizer_cxx" \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
  cmake --build build-sanitize --target nightfall_core_sanitizer_smoke
  ASAN_OPTIONS=detect_leaks=0 timeout 10 build-sanitize/nightfall_core_sanitizer_smoke
}

run_nflint() {
  local target="$1"
  if rg -n '\bprintf\s*\(' "$target" >/tmp/nightfall-ci.err; then
    return 1
  fi
  if rg -n '\b(qemu|Basilisk II|SheepShaver|Wine|Executor|PearPC|Dolphin)\b' "$target" >/tmp/nightfall-ci.err; then
    return 1
  fi
}

run_boundary_check() {
  local target="$1"
  if rg -n '#[[:space:]]*include[[:space:]]*[<"](AppKit|Foundation|Metal)' "$target" >/tmp/nightfall-ci.err; then
    return 1
  fi
}

run_tbcover() {
  local dir="$1"
  local src base missing=0
  while IFS= read -r src; do
    base="${src%.cpp}"
    [ -f "$base.test.cpp" ] || { printf '%s missing .test.cpp\n' "$src" >&2; missing=1; }
    [ -f "$base.md" ] || { printf '%s missing .md\n' "$src" >&2; missing=1; }
    [ -f "$base.fixture.s" ] || { printf '%s missing .fixture.s\n' "$src" >&2; missing=1; }
  done < <(find "$dir" -maxdepth 1 -name '*.cpp' ! -name '*.test.cpp' -print)
  [ "$missing" -eq 0 ]
}

run_asm_fixture() {
  local fixture="$1"
  local out
  out="$(mktemp -t nightfall_fixture_XXXXXX)"
  local log
  log="$(mktemp -t nightfall_fixture_log_XXXXXX)"
  vasmm68k_mot -quiet -Fbin -o "$out" "$fixture" >"$log" 2>&1
  if grep -Eiq '(^|[^[:alpha:]])(fatal|error)[^[:alpha:]]' "$log"; then
    cat "$log" >&2
    rm -f "$out" "$log"
    return 1
  fi
  rm -f "$log"
  rm -f "$out"
}

run_depcheck() {
  local file="$1"
  ! grep -Eq 'TODO|FIXME|XXX|<pin>|""[[:space:]]*$' "$file" || return 1
  grep -Eq 'bootstrap_policy_accepted[[:space:]]*=[[:space:]]*true' "$file" || return 1
  grep -Eq '^\[dependencies\.musashi\]' "$file" || return 1
  grep -Eq '^\[dependencies\.nlohmann_json\]' "$file" || return 1
  grep -Eq '^\[dependencies\.glsl_crt\]' "$file" || return 1
  grep -Eq '^\[tools\.vasm\]' "$file" || return 1
  grep -Eq '^\[tools\.libabigail\]' "$file" || return 1
}

run_trace_schema_guard() {
  local dir="$1"
  local schema="$dir/trace-events.toml"
  [ -f "$schema" ] || return 1
  while IFS= read -r call; do
    local category name
    category="$(printf '%s\n' "$call" | sed -E 's/.*emit_trace\([^,]+,[[:space:]]*"([^"]+)".*/\1/')"
    name="$(printf '%s\n' "$call" | sed -E 's/.*emit_trace\([^,]+,[[:space:]]*"[^"]+",[[:space:]]*"([^"]+)".*/\1/')"
    grep -Eq "^\[events\\.\"$name\"\\]" "$schema" || return 1
    grep -Eq "category[[:space:]]*=[[:space:]]*\"$category\"" "$schema" || return 1
  done < <(rg 'emit_trace\([^,]+,[[:space:]]*"[^"]+",[[:space:]]*"[^"]+"' "$dir" -g '*.cpp')
}

run_abi_guard() {
  local dir="$1"
  cmp -s "$dir/nightfall.h" "$dir/nightfall.h.snapshot" || return 1
}

require_tool cmake
require_tool ctest
require_tool rg
require_tool vasmm68k_mot

expect_success "build-and-tests" run_build_and_tests
expect_success "sanitizers" run_sanitizers

expect_success "nflint good fixture" run_nflint tests/gates/nflint/good
expect_failure "nflint printf fixture" run_nflint tests/gates/nflint/bad/Printf.cpp
expect_failure "nflint forbidden-source fixture" run_nflint tests/gates/nflint/bad/ForbiddenSource.cpp
expect_success "nflint core" run_nflint core

expect_success "boundary good fixture" run_boundary_check tests/gates/boundary/good
expect_failure "boundary bad fixture" run_boundary_check tests/gates/boundary/bad
expect_success "boundary core" run_boundary_check core

expect_success "tbcover good fixture" run_tbcover tests/gates/tbcover/good
expect_failure "tbcover missing doc fixture" run_tbcover tests/gates/tbcover/bad
expect_success "tbcover core" run_tbcover core

expect_success "asm good fixture" run_asm_fixture tests/gates/asm-fixtures/good/smoke.fixture.s
expect_failure "asm bad fixture" run_asm_fixture tests/gates/asm-fixtures/bad/broken.fixture.s
expect_success "asm core fixture" run_asm_fixture core/nightfall.fixture.s
expect_success "asm sanitizer fixture" run_asm_fixture core/nightfall.sanitizer.fixture.s

expect_success "depcheck good fixture" run_depcheck tests/gates/depcheck/good/VERSIONS.toml
expect_failure "depcheck missing policy fixture" run_depcheck tests/gates/depcheck/bad/MissingPolicy.toml
expect_failure "depcheck placeholder fixture" run_depcheck tests/gates/depcheck/bad/Placeholder.toml
expect_success "depcheck project versions" run_depcheck third_party/VERSIONS.toml

expect_success "trace schema good fixture" run_trace_schema_guard tests/gates/trace-schema/good
expect_failure "trace schema missing event fixture" run_trace_schema_guard tests/gates/trace-schema/bad

expect_success "abi guard good fixture" run_abi_guard tests/gates/abi-guard/good
expect_failure "abi guard bad fixture" run_abi_guard tests/gates/abi-guard/bad

expect_success "sanitizer meta good fixture" tests/gates/sanitizers/good/run.sh
expect_failure "sanitizer meta bad fixture" tests/gates/sanitizers/bad/run.sh

pass "local gate suite"
