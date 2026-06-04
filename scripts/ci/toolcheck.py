#!/usr/bin/env python3
"""Validate required local tools are available on PATH."""

from __future__ import annotations

import argparse
import shutil
import sys


REQUIRED_TOOL_GROUPS = {
    "git": ["git"],
    "cmake": ["cmake"],
    "clang": ["clang"],
    "python3": ["python3"],
    "vasm-family": ["vasm", "vasmm68k_mot", "vasmm68k_std", "vasmm68k"],
}

OPTIONAL_TOOL_GROUPS = {
    "gh": ["gh"],
    "abidiff": ["abidiff"],
}


def found_any(candidates: list[str]) -> str | None:
    for candidate in candidates:
        path = shutil.which(candidate)
        if path is not None:
            return path
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--include-optional", action="store_true")
    args = parser.parse_args()

    errors: list[str] = []
    for name, candidates in REQUIRED_TOOL_GROUPS.items():
        path = found_any(candidates)
        if path is None:
            errors.append(f"missing required tool group '{name}' ({', '.join(candidates)})")
        else:
            print(f"PASS  {name}: {path}")

    if args.include_optional:
        for name, candidates in OPTIONAL_TOOL_GROUPS.items():
            path = found_any(candidates)
            if path is None:
                errors.append(f"missing optional tool group requested as required '{name}'")
            else:
                print(f"PASS  {name}: {path}")

    for error in errors:
        print(error, file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
