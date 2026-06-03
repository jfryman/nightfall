#!/usr/bin/env bash
set -euo pipefail

: "${NF_BUILD_DIR:=build}"

cmake -S . -B "$NF_BUILD_DIR"
cmake --build "$NF_BUILD_DIR"
ctest --test-dir "$NF_BUILD_DIR" --output-on-failure
scripts/ci/sanitizers.sh

scripts/ci/abi-guard.sh
scripts/ci/boundary-check.py core
scripts/ci/nflint.py core
scripts/ci/depcheck.py third_party/VERSIONS.toml
scripts/ci/toolcheck.py
scripts/ci/trace-schema-guard.py --schema docs/trace-events.toml core
scripts/ci/tbcover.py core
scripts/ci/asm-fixtures.sh
scripts/ci/gate-meta-tests.sh
