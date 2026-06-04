#!/usr/bin/env python3
"""Reject app/framework imports inside core sources."""

from __future__ import annotations

import argparse
import pathlib
import re
import sys


FORBIDDEN_PATTERNS = [
    re.compile(r"^\s*#\s*include\s*[<\"](?:AppKit|Foundation|Metal)/", re.MULTILINE),
    re.compile(r"^\s*@import\s+(?:AppKit|Foundation|Metal)\b", re.MULTILINE),
    re.compile(r"^\s*import\s+(?:AppKit|Foundation|Metal)\b", re.MULTILINE),
]


def iter_sources(root: pathlib.Path) -> list[pathlib.Path]:
    suffixes = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".m", ".mm", ".swift"}
    return sorted(path for path in root.rglob("*") if path.is_file() and path.suffix in suffixes)


def check_file(path: pathlib.Path) -> list[str]:
    text = path.read_text(encoding="utf-8", errors="ignore")
    errors: list[str] = []
    for pattern in FORBIDDEN_PATTERNS:
        if pattern.search(text):
            errors.append(f"{path}: forbidden app/framework import in core boundary")
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
