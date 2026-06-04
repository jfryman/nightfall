#!/usr/bin/env python3
"""Generate the Phase 4.6 PICT 2 fixture slice from a small YAML-like spec."""

from __future__ import annotations

import argparse
import pathlib
import re
import sys


OPCODES = {
    "nop": 0x0000,
    "rgb_fore_color": 0x001A,
    "rgb_back_color": 0x001B,
    "line": 0x0020,
    "frame_rect": 0x0030,
    "paint_rect": 0x0031,
    "erase_rect": 0x0032,
    "invert_rect": 0x0033,
    "fill_rect": 0x0034,
}

RECT_OPS = {"frame_rect", "paint_rect", "erase_rect", "invert_rect", "fill_rect"}
RGB_OPS = {"rgb_fore_color", "rgb_back_color"}


class SpecError(ValueError):
    pass


def _strip_comment(line: str) -> str:
    return line.split("#", 1)[0].strip()


def _parse_int_list(raw: str, expected: int, name: str) -> list[int]:
    match = re.fullmatch(r"\[([^\]]*)\]", raw.strip())
    if not match:
        raise SpecError(f"{name}: expected [a, b, ...]")
    values = [item.strip() for item in match.group(1).split(",") if item.strip()]
    if len(values) != expected:
        raise SpecError(f"{name}: expected {expected} integers, got {len(values)}")
    try:
        return [int(value, 0) for value in values]
    except ValueError as exc:
        raise SpecError(f"{name}: invalid integer") from exc


def parse_spec(text: str) -> dict[str, object]:
    frame: list[int] | None = None
    operations: list[tuple[str, list[int]]] = []
    in_ops = False

    for line_number, original in enumerate(text.splitlines(), start=1):
        line = _strip_comment(original)
        if not line:
            continue
        if line == "ops:":
            in_ops = True
            continue
        if line.startswith("frame:"):
            frame = _parse_int_list(line.split(":", 1)[1], 4, f"line {line_number} frame")
            continue
        if in_ops and line.startswith("- "):
            item = line[2:].strip()
            if ":" in item:
                name, raw_values = [part.strip() for part in item.split(":", 1)]
                expected = 3 if name in RGB_OPS else 4 if name in RECT_OPS else 0
                if name == "line":
                    expected = 4
                values = _parse_int_list(raw_values, expected, f"line {line_number} {name}")
            else:
                name = item
                values = []
            if name not in OPCODES:
                raise SpecError(f"line {line_number}: unknown operation {name}")
            operations.append((name, values))
            continue
        raise SpecError(f"line {line_number}: unsupported spec line {original!r}")

    if frame is None:
        raise SpecError("missing frame")
    return {"frame": frame, "ops": operations}


def _u16(value: int) -> bytes:
    if value < -32768 or value > 65535:
        raise SpecError(f"value out of 16-bit range: {value}")
    return int(value & 0xFFFF).to_bytes(2, "big")


def _u32(value: int) -> bytes:
    return int(value & 0xFFFFFFFF).to_bytes(4, "big")


def _rect(values: list[int]) -> bytes:
    return b"".join(_u16(value) for value in values)


def _fixed_rect(values: list[int]) -> bytes:
    return b"".join(_u32(value << 16) for value in values)


def emit_picture(spec: dict[str, object]) -> bytes:
    frame = spec["frame"]
    assert isinstance(frame, list)
    ops = spec["ops"]
    assert isinstance(ops, list)

    data = bytearray()
    data.extend(_u16(0))
    data.extend(_rect(frame))
    data.extend(_u16(0x0011))
    data.extend(_u16(0x02FF))
    data.extend(_u16(0x0C00))
    data.extend(_u32(-1))
    data.extend(_fixed_rect(frame))
    data.extend(_u32(0))

    for name, values in ops:
        data.extend(_u16(OPCODES[name]))
        if name in RGB_OPS:
            for value in values:
                data.extend(_u16(value))
        elif name in RECT_OPS or name == "line":
            data.extend(_rect(values))
        elif name == "nop":
            pass
        else:
            raise SpecError(f"unsupported operation {name}")

    data.extend(_u16(0x00FF))
    data[0:2] = _u16(min(len(data), 0xFFFF))
    return bytes(data)


def coverage_for(spec: dict[str, object]) -> list[str]:
    ops = spec["ops"]
    assert isinstance(ops, list)
    names = ["version", "header"]
    names.extend(name for name, _values in ops)
    names.append("end_pic")
    return names


def write_rez(path: pathlib.Path, picture: bytes, resource_id: int) -> None:
    chunks = [picture[index:index + 16] for index in range(0, len(picture), 16)]
    lines = [f"data 'PICT' ({resource_id}) {{\n"]
    for chunk in chunks:
        lines.append('  $"' + chunk.hex().upper() + '"\n')
    lines.append("};\n")
    path.write_text("".join(lines), encoding="utf-8")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("spec", type=pathlib.Path)
    parser.add_argument("output", type=pathlib.Path)
    parser.add_argument("--coverage", action="store_true", help="print covered opcode names")
    parser.add_argument("--rez-output", type=pathlib.Path, help="write a Rez-style PICT resource source")
    parser.add_argument("--resource-id", type=int, default=128)
    args = parser.parse_args(argv)

    try:
        spec = parse_spec(args.spec.read_text(encoding="utf-8"))
        picture = emit_picture(spec)
    except SpecError as exc:
        print(f"pict-gen: {exc}", file=sys.stderr)
        return 2

    args.output.write_bytes(picture)
    if args.rez_output is not None:
        write_rez(args.rez_output, picture, args.resource_id)
    if args.coverage:
        for name in coverage_for(spec):
            print(name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
