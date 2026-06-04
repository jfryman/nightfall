#!/usr/bin/env python3
"""Ensure emitted trace events are declared in the trace schema."""

from __future__ import annotations

import argparse
import pathlib
import re
import sys
import tomllib


TRACE_CALL_PATTERN = re.compile(
    r'emit_trace\s*\([^;]*?"(?P<category>[^"]+)"\s*,\s*"(?P<name>[^"]+)"',
    re.DOTALL,
)


def iter_sources(root: pathlib.Path) -> list[pathlib.Path]:
    suffixes = {".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp"}
    return sorted(path for path in root.rglob("*") if path.is_file() and path.suffix in suffixes)


def load_schema(path: pathlib.Path) -> dict[str, str]:
    data = tomllib.loads(path.read_text(encoding="utf-8"))
    events = data.get("events", {})
    if not isinstance(events, dict):
        raise ValueError("trace schema must define an [events] table")

    schema: dict[str, str] = {}
    for name, value in events.items():
        if not isinstance(value, dict) or not isinstance(value.get("category"), str):
            raise ValueError(f"event '{name}' must define a string category")
        schema[name] = value["category"]
    return schema


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--schema", default="docs/trace-events.toml")
    parser.add_argument("paths", nargs="*", default=["core"])
    args = parser.parse_args()

    try:
        schema = load_schema(pathlib.Path(args.schema))
    except (OSError, tomllib.TOMLDecodeError, ValueError) as error:
        print(f"{args.schema}: {error}", file=sys.stderr)
        return 1

    errors: list[str] = []
    seen: set[str] = set()
    for raw_path in args.paths:
        path = pathlib.Path(raw_path)
        sources = iter_sources(path) if path.is_dir() else [path]
        for source in sources:
            text = source.read_text(encoding="utf-8", errors="ignore")
            for match in TRACE_CALL_PATTERN.finditer(text):
                category = match.group("category")
                name = match.group("name")
                expected_category = schema.get(name)
                seen.add(name)
                if expected_category is None:
                    errors.append(f"{source}: trace event '{name}' is not declared in {args.schema}")
                elif expected_category != category:
                    errors.append(
                        f"{source}: trace event '{name}' category '{category}' "
                        f"does not match schema category '{expected_category}'"
                    )

    for error in errors:
        print(error, file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
