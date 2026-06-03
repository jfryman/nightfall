#!/usr/bin/env python3
"""Validate the dependency manifest shape used by the local gate loop."""

from __future__ import annotations

import argparse
import pathlib
import sys
import tomllib


REQUIRED_TABLES = [
    ("policy",),
    ("dependencies", "musashi"),
    ("dependencies", "nlohmann_json"),
    ("dependencies", "glsl_crt"),
    ("tools", "vasm"),
    ("tools", "libabigail"),
    ("toolchain", "doctest"),
    ("toolchain", "libfuzzer"),
    ("toolchain", "sanitizers"),
    ("toolchain", "xcodegen"),
    ("build_host",),
]


def table_at(data: dict, path: tuple[str, ...]) -> object | None:
    current: object = data
    for part in path:
        if not isinstance(current, dict) or part not in current:
            return None
        current = current[part]
    return current


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("manifest", nargs="?", default="third_party/VERSIONS.toml")
    parser.add_argument("--require-exact-pins", action="store_true")
    args = parser.parse_args()

    manifest = pathlib.Path(args.manifest)
    if not manifest.exists():
        print(f"{manifest}: dependency manifest is missing", file=sys.stderr)
        return 1

    text = manifest.read_text(encoding="utf-8")
    if any(marker in text for marker in ("TODO", "FIXME", "XXX", "<pin>")):
        print(f"{manifest}: unfilled placeholder marker found", file=sys.stderr)
        return 1

    try:
        data = tomllib.loads(text)
    except tomllib.TOMLDecodeError as error:
        print(f"{manifest}: invalid TOML: {error}", file=sys.stderr)
        return 1

    errors: list[str] = []
    for table_path in REQUIRED_TABLES:
        if table_at(data, table_path) is None:
            errors.append(f"{manifest}: missing table [{'.'.join(table_path)}]")

    policy = table_at(data, ("policy",))
    if isinstance(policy, dict):
        if policy.get("bootstrap_policy_accepted") is not True:
            errors.append(f"{manifest}: policy.bootstrap_policy_accepted must be true")
        exact_pins = policy.get("exact_pins_materialized")
        if args.require_exact_pins and exact_pins is not True:
            errors.append(f"{manifest}: exact dependency pins are required for this mode")
        elif exact_pins is not True:
            print(f"WARN  {manifest}: exact pins not materialized yet", file=sys.stderr)

    for error in errors:
        print(error, file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
