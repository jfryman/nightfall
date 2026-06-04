#include "doctest.h"

#include "QuickDrawPicture.h"

#include <stdint.h>
#include <vector>

namespace {

void push_u16(std::vector<uint8_t> *bytes, uint16_t value) {
  bytes->push_back(static_cast<uint8_t>((value >> 8u) & 0xFFu));
  bytes->push_back(static_cast<uint8_t>(value & 0xFFu));
}

void push_rect(std::vector<uint8_t> *bytes, int16_t top, int16_t left, int16_t bottom, int16_t right) {
  push_u16(bytes, static_cast<uint16_t>(top));
  push_u16(bytes, static_cast<uint16_t>(left));
  push_u16(bytes, static_cast<uint16_t>(bottom));
  push_u16(bytes, static_cast<uint16_t>(right));
}

std::vector<uint8_t> minimal_picture() {
  std::vector<uint8_t> bytes;
  push_u16(&bytes, 0u);
  push_rect(&bytes, 0, 0, 16, 16);
  push_u16(&bytes, nightfall::quickdraw::kPictVersionOp);
  push_u16(&bytes, nightfall::quickdraw::kPictVersion2);
  push_u16(&bytes, nightfall::quickdraw::kPictHeaderOp);
  for (int index = 0; index < 24; ++index) {
    bytes.push_back(index < 4 ? 0xFFu : 0u);
  }
  return bytes;
}

}  // namespace

TEST_CASE("QuickDraw PICT validator records initial PICT 2 opcode coverage") {
  std::vector<uint8_t> bytes = minimal_picture();
  push_u16(&bytes, nightfall::quickdraw::kPictRGBForeColor);
  push_u16(&bytes, 0xFFFFu);
  push_u16(&bytes, 0u);
  push_u16(&bytes, 0u);
  push_u16(&bytes, nightfall::quickdraw::kPictLine);
  push_rect(&bytes, 0, 0, 15, 15);
  push_u16(&bytes, nightfall::quickdraw::kPictPaintRect);
  push_rect(&bytes, 2, 2, 8, 8);
  push_u16(&bytes, nightfall::quickdraw::kPictEndPic);

  nightfall::quickdraw::PictOpcodeCoverage coverage{};
  CHECK(nightfall::quickdraw::validate_pict2(bytes.data(), bytes.size(), &coverage) == NF_OK);

  CHECK(coverage.frame.bottom == 16);
  CHECK(coverage.frame.right == 16);
  CHECK(coverage.saw_version);
  CHECK(coverage.saw_header);
  CHECK(coverage.saw_rgb_fore_color);
  CHECK(coverage.saw_line);
  CHECK(coverage.saw_paint_rect);
  CHECK(coverage.saw_end_pic);
  CHECK(coverage.opcode_count == 6u);
}

TEST_CASE("QuickDraw PICT validator rejects truncated and unsupported streams") {
  std::vector<uint8_t> bytes = minimal_picture();
  push_u16(&bytes, nightfall::quickdraw::kPictPaintRect);
  push_rect(&bytes, 0, 0, 4, 4);

  CHECK(nightfall::quickdraw::validate_pict2(bytes.data(), bytes.size(), nullptr) ==
        NF_ERROR_INCOMPLETE_PROGRAM);

  bytes.push_back(0u);
  CHECK(nightfall::quickdraw::validate_pict2(bytes.data(), bytes.size(), nullptr) ==
        NF_ERROR_INVALID_ARGUMENT);

  std::vector<uint8_t> unsupported = minimal_picture();
  push_u16(&unsupported, 0x7777u);
  CHECK(nightfall::quickdraw::validate_pict2(unsupported.data(), unsupported.size(), nullptr) ==
        NF_ERROR_UNIMPLEMENTED);
}
