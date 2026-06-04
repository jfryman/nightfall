#!/usr/bin/env python3

from __future__ import annotations

import importlib.util
import pathlib
import tempfile
import unittest


MODULE_PATH = pathlib.Path(__file__).with_name("pict-gen.py")
SPEC = importlib.util.spec_from_file_location("pict_gen", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
pict_gen = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(pict_gen)


class PictGenTests(unittest.TestCase):
    def test_emits_version_2_header_and_covered_opcodes(self) -> None:
        spec = pict_gen.parse_spec(
            """
            frame: [0, 0, 16, 16]
            ops:
              - rgb_fore_color: [65535, 0, 0]
              - line: [0, 0, 15, 15]
              - paint_rect: [2, 2, 8, 8]
            """
        )

        picture = pict_gen.emit_picture(spec)

        self.assertEqual(picture[10:14], bytes.fromhex("001102FF"))
        self.assertEqual(picture[14:16], bytes.fromhex("0C00"))
        self.assertIn(bytes.fromhex("001AFFFF00000000"), picture)
        self.assertIn(bytes.fromhex("002000000000000F000F"), picture)
        self.assertTrue(picture.endswith(bytes.fromhex("00FF")))
        self.assertEqual(
            pict_gen.coverage_for(spec),
            ["version", "header", "rgb_fore_color", "line", "paint_rect", "end_pic"],
        )

    def test_rez_output_wraps_picture_resource(self) -> None:
        spec = pict_gen.parse_spec(
            """
            frame: [0, 0, 4, 4]
            ops:
              - nop
            """
        )
        picture = pict_gen.emit_picture(spec)
        with tempfile.TemporaryDirectory() as raw_tmp:
            rez_path = pathlib.Path(raw_tmp) / "fixture.r"
            pict_gen.write_rez(rez_path, picture, 129)
            rez = rez_path.read_text(encoding="utf-8")

        self.assertIn("data 'PICT' (129)", rez)
        self.assertIn(picture[:8].hex().upper(), rez)


if __name__ == "__main__":
    unittest.main()
