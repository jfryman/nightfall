#!/usr/bin/env bash
set -uo pipefail

failures=0

pass() { printf 'PASS  %s\n' "$1"; }
fail() { printf 'FAIL  %s\n' "$1" >&2; failures=$((failures + 1)); }

expect_pass() {
  local name="$1"
  shift
  if "$@" >/tmp/nightfall-gate-pass.out 2>/tmp/nightfall-gate-pass.err; then
    pass "$name"
  else
    fail "$name"
    sed 's/^/      /' /tmp/nightfall-gate-pass.err >&2
  fi
}

expect_fail_with() {
  local name="$1"
  local needle="$2"
  shift 2
  if "$@" >/tmp/nightfall-gate-fail.out 2>/tmp/nightfall-gate-fail.err; then
    fail "$name (unexpected pass)"
  elif grep -q "$needle" /tmp/nightfall-gate-fail.err; then
    pass "$name"
  else
    fail "$name (failed for wrong reason)"
    sed 's/^/      /' /tmp/nightfall-gate-fail.err >&2
  fi
}

expect_pass \
  "boundary-check accepts core-only fixture" \
  scripts/ci/boundary-check.py tests/gates/boundary/good

expect_fail_with \
  "boundary-check rejects Foundation fixture" \
  "forbidden app/framework import" \
  scripts/ci/boundary-check.py tests/gates/boundary/bad

expect_pass \
  "nflint accepts trace-only fixture" \
  scripts/ci/nflint.py tests/gates/nflint/good

expect_fail_with \
  "nflint rejects printf fixture" \
  "printf-style output" \
  scripts/ci/nflint.py tests/gates/nflint/bad/Printf.cpp

expect_fail_with \
  "nflint rejects forbidden-source fixture" \
  "forbidden-source provenance token" \
  scripts/ci/nflint.py tests/gates/nflint/bad/ForbiddenSource.cpp

expect_pass \
  "depcheck accepts complete manifest fixture" \
  scripts/ci/depcheck.py --require-exact-pins tests/gates/depcheck/good/VERSIONS.toml

expect_fail_with \
  "depcheck rejects missing policy fixture" \
  "missing table \\[policy\\]" \
  scripts/ci/depcheck.py tests/gates/depcheck/bad/MissingPolicy.toml

expect_fail_with \
  "depcheck rejects placeholder fixture" \
  "unfilled placeholder marker" \
  scripts/ci/depcheck.py tests/gates/depcheck/bad/Placeholder.toml

expect_pass \
  "toolcheck accepts complete fake PATH" \
  env PATH="$PWD/tests/gates/toolcheck/fakebin/good:/bin:/usr/bin" scripts/ci/toolcheck.py

expect_fail_with \
  "toolcheck rejects missing assembler fixture" \
  "missing required tool group 'vasm-family'" \
  env PATH="$PWD/tests/gates/toolcheck/fakebin/bad:/bin:/usr/bin" scripts/ci/toolcheck.py

expect_pass \
  "trace-schema-guard accepts declared event fixture" \
  scripts/ci/trace-schema-guard.py \
    --schema tests/gates/trace-schema/good/trace-events.toml \
    tests/gates/trace-schema/good

expect_fail_with \
  "trace-schema-guard rejects undeclared event fixture" \
  "is not declared" \
  scripts/ci/trace-schema-guard.py \
    --schema tests/gates/trace-schema/good/trace-events.toml \
    tests/gates/trace-schema/bad/MissingEvent.cpp

expect_fail_with \
  "trace-schema-guard rejects wrong category fixture" \
  "does not match schema category" \
  scripts/ci/trace-schema-guard.py \
    --schema tests/gates/trace-schema/good/trace-events.toml \
    tests/gates/trace-schema/bad/WrongCategory.cpp

expect_pass \
  "tbcover accepts documented tested fixture" \
  scripts/ci/tbcover.py tests/gates/tbcover/good

expect_fail_with \
  "tbcover rejects missing test fixture" \
  "missing co-located test" \
  scripts/ci/tbcover.py tests/gates/tbcover/bad/MissingTest.cpp

expect_fail_with \
  "tbcover rejects missing doc fixture" \
  "missing co-located documentation" \
  scripts/ci/tbcover.py tests/gates/tbcover/bad/MissingDoc.cpp

expect_fail_with \
  "tbcover rejects trap unit missing fixture" \
  "missing co-located fixture" \
  scripts/ci/tbcover.py tests/gates/tbcover/bad/MissingFixture.cpp

expect_pass \
  "asm-fixtures assembles smoke fixture" \
  env NF_FIXTURE_BUILD_DIR=/tmp/nightfall-asm-good scripts/ci/asm-fixtures.sh

expect_fail_with \
  "asm-fixtures rejects broken fixture" \
  "unknown mnemonic" \
  env NF_FIXTURE_BUILD_DIR=/tmp/nightfall-asm-bad \
    NF_ASM_INPUT=tests/gates/asm-fixtures/bad/broken.fixture.s \
    scripts/ci/asm-fixtures.sh

expect_pass \
  "abi-guard accepts current ABI snapshots" \
  scripts/ci/abi-guard.sh

expect_fail_with \
  "abi-guard rejects header drift fixture" \
  "public ABI header differs from snapshot" \
  env NF_ABI_HEADER=tests/gates/abi-guard/bad/nightfall.changed.h \
    NF_ABI_HEADER_SNAPSHOT=tests/gates/abi-guard/bad/nightfall.h.snapshot \
    scripts/ci/abi-guard.sh

expect_fail_with \
  "abi-guard rejects symbol drift fixture" \
  "exported ABI symbols differ from snapshot" \
  env NF_ABI_SYMBOLS=tests/gates/abi-guard/bad/symbols.expected \
    scripts/ci/abi-guard.sh

expect_pass \
  "sanitizer meta fixture accepts passing command" \
  tests/gates/sanitizers/good/run.sh

expect_fail_with \
  "sanitizer meta fixture rejects failing command" \
  "sanitizer fixture failure" \
  tests/gates/sanitizers/bad/run.sh

exit "$failures"
