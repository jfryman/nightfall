#!/usr/bin/env bash
set -euo pipefail

: "${NF_ASM:=vasmm68k_mot}"
: "${NF_FIXTURE_BUILD_DIR:=build/fixtures}"

mkdir -p "$NF_FIXTURE_BUILD_DIR"

if [ -n "${NF_ASM_INPUT:-}" ]; then
  fixture_list="$NF_ASM_INPUT"
else
  fixture_list="$(find core -name '*.fixture.s' -type f)"
fi

for fixture in $fixture_list; do
  [ -e "$fixture" ] || continue
  base="$(printf '%s' "$fixture" | sed 's#[/.]#_#g; s#_fixture_s$##')"
  "$NF_ASM" -Fbin -o "$NF_FIXTURE_BUILD_DIR/$base.bin" "$fixture" >/dev/null
done
