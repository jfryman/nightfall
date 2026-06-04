#!/usr/bin/env python3
"""Nightfall source lint checks for early Phase 0 enforcement."""

from __future__ import annotations

import argparse
import pathlib
import re
import sys


FORBIDDEN_SOURCE_NAMES = [
    "basilisk",
    "sheepshaver",
    "executor",
    "pearpc",
    "qemu",
    "dolphin",
]

PRINTF_PATTERN = re.compile(r"\b(?:printf|fprintf|puts|std::cout|std::cerr)\b")


def iter_sources(root: pathlib.Path) -> list[pathlib.Path]:
    suffixes = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".m", ".mm"}
    return sorted(path for path in root.rglob("*") if path.is_file() and path.suffix in suffixes)


def check_file(path: pathlib.Path) -> list[str]:
    text = path.read_text(encoding="utf-8", errors="ignore")
    errors: list[str] = []

    if PRINTF_PATTERN.search(text):
        errors.append(f"{path}: printf-style output is forbidden in core; use nf_trace_event")

    lowered = text.lower()
    for name in FORBIDDEN_SOURCE_NAMES:
        if name in lowered:
            errors.append(f"{path}: forbidden-source provenance token '{name}' found")

    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="*", default=["core"])
    args = parser.parse_args()

    errors: list[str] = []
    for raw_path in args.paths:
        path = pathlib.Path(raw_path)
        if path.is_dir():
            for source in iter_sources(path):
                errors.extend(check_file(source))
        elif path.exists():
            errors.extend(check_file(path))

    for error in errors:
        print(error, file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
