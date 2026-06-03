#!/usr/bin/env bash
set -euo pipefail

: "${NF_BUILD_DIR:=build}"
: "${NF_ABI_HEADER:=core/nightfall.h}"
: "${NF_ABI_HEADER_SNAPSHOT:=docs/abi/nightfall.h.snapshot}"
: "${NF_ABI_SYMBOLS:=docs/abi/nightfall.symbols}"
: "${NF_ABI_LIBRARY:=$NF_BUILD_DIR/libnightfall_core.a}"

tmp_symbols="$(mktemp -t nightfall-abi-symbols.XXXXXX)"
trap 'rm -f "$tmp_symbols"' EXIT

if [ ! -f "$NF_ABI_HEADER" ]; then
  echo "$NF_ABI_HEADER: ABI header missing" >&2
  exit 1
fi

if [ ! -f "$NF_ABI_HEADER_SNAPSHOT" ]; then
  echo "$NF_ABI_HEADER_SNAPSHOT: ABI header snapshot missing" >&2
  exit 1
fi

if ! diff -u "$NF_ABI_HEADER_SNAPSHOT" "$NF_ABI_HEADER" >/tmp/nightfall-abi-header.diff; then
  echo "$NF_ABI_HEADER: public ABI header differs from snapshot $NF_ABI_HEADER_SNAPSHOT" >&2
  cat /tmp/nightfall-abi-header.diff >&2
  exit 1
fi

if [ ! -f "$NF_ABI_LIBRARY" ]; then
  echo "$NF_ABI_LIBRARY: ABI library missing; build before running abi-guard" >&2
  exit 1
fi

if [ ! -f "$NF_ABI_SYMBOLS" ]; then
  echo "$NF_ABI_SYMBOLS: ABI symbol snapshot missing" >&2
  exit 1
fi

nm -gU "$NF_ABI_LIBRARY" \
  | awk '{print $NF}' \
  | sed 's/^_//' \
  | grep '^nf_' \
  | sort -u >"$tmp_symbols"

if ! diff -u "$NF_ABI_SYMBOLS" "$tmp_symbols" >/tmp/nightfall-abi-symbols.diff; then
  echo "$NF_ABI_LIBRARY: exported ABI symbols differ from snapshot $NF_ABI_SYMBOLS" >&2
  cat /tmp/nightfall-abi-symbols.diff >&2
  exit 1
fi
