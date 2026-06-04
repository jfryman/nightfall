#!/usr/bin/env python3
"""Check core source co-location for tests, docs, and fixture-backed units."""

from __future__ import annotations

import argparse
import pathlib
import sys


IMPLEMENTATION_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}


def is_implementation(path: pathlib.Path) -> bool:
    if path.suffix not in IMPLEMENTATION_SUFFIXES:
        return False
    return not path.name.endswith(".test.cpp")


def has_sibling(stem: pathlib.Path, suffix: str) -> bool:
    return stem.with_name(stem.name + suffix).exists()


def check_unit(path: pathlib.Path) -> list[str]:
    stem = path.with_suffix("")
    errors: list[str] = []

    if not has_sibling(stem, ".test.cpp"):
        errors.append(f"{path}: missing co-located test {stem.name}.test.cpp")

    if not has_sibling(stem, ".md"):
        errors.append(f"{path}: missing co-located documentation {stem.name}.md")

    text = path.read_text(encoding="utf-8", errors="ignore")
    mentions_trap = "trap" in text.lower()
    has_fixture = has_sibling(stem, ".fixture.s")
    if mentions_trap and not has_fixture:
        errors.append(f"{path}: trap-related unit missing co-located fixture {stem.name}.fixture.s")

    return errors


def iter_units(root: pathlib.Path) -> list[pathlib.Path]:
    if root.is_file():
        return [root] if is_implementation(root) else []
    return sorted(path for path in root.rglob("*") if path.is_file() and is_implementation(path))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="*", default=["core"])
    args = parser.parse_args()

    errors: list[str] = []
    for raw_path in args.paths:
        path = pathlib.Path(raw_path)
        if path.exists():
            for unit in iter_units(path):
                errors.extend(check_unit(unit))

    for error in errors:
        print(error, file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
