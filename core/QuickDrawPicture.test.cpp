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

std::vector<uint8_t> playback_picture() {
  std::vector<uint8_t> bytes = minimal_picture();
  push_u16(&bytes, nightfall::quickdraw::kPictRGBForeColor);
  push_u16(&bytes, 0xFFFFu);
  push_u16(&bytes, 0u);
  push_u16(&bytes, 0u);
  push_u16(&bytes, nightfall::quickdraw::kPictPaintRect);
  push_rect(&bytes, 2, 2, 8, 8);
  push_u16(&bytes, nightfall::quickdraw::kPictLine);
  push_rect(&bytes, 0, 0, 15, 15);
  push_u16(&bytes, nightfall::quickdraw::kPictRGBBackColor);
  push_u16(&bytes, 0u);
  push_u16(&bytes, 0u);
  push_u16(&bytes, 0xFFFFu);
  push_u16(&bytes, nightfall::quickdraw::kPictEraseRect);
  push_rect(&bytes, 8, 8, 12, 12);
  push_u16(&bytes, nightfall::quickdraw::kPictEndPic);
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

TEST_CASE("QuickDraw DrawPicture replays covered PICT 2 opcodes into the current port") {
  nightfall::quickdraw::PortState state;
  CHECK(state.init_graf(0x00001000u) == NF_OK);
  CHECK(state.open_port(0x00002000u) == NF_OK);
  std::vector<uint8_t> bytes = playback_picture();

  nightfall::quickdraw::PictOpcodeCoverage coverage{};
  CHECK(state.draw_picture(bytes.data(), bytes.size(), nightfall::quickdraw::Rect{0, 0, 16, 16}, &coverage) ==
        NF_OK);

  CHECK(state.pixel(0x00002000u, 2, 2) == 0x00FF0000u);
  CHECK(state.pixel(0x00002000u, 7, 7) == 0x00FF0000u);
  CHECK(state.pixel(0x00002000u, 8, 8) == 0x000000FFu);
  CHECK(state.pixel(0x00002000u, 11, 11) == 0x000000FFu);
  CHECK(state.pixel(0x00002000u, 15, 15) == 0x00FF0000u);
  CHECK(coverage.saw_rgb_fore_color);
  CHECK(coverage.saw_paint_rect);
  CHECK(coverage.saw_line);
  CHECK(coverage.saw_rgb_back_color);
  CHECK(coverage.saw_erase_rect);
  CHECK(coverage.saw_end_pic);
  CHECK(state.trace_word(3u) == nightfall::quickdraw::kTrapDrawPicture);
}

TEST_CASE("QuickDraw DrawPicture maps picture frame coordinates into the destination rectangle") {
  nightfall::quickdraw::PortState state;
  CHECK(state.init_graf(0x00001000u) == NF_OK);
  CHECK(state.open_port(0x00002000u) == NF_OK);
  std::vector<uint8_t> bytes = minimal_picture();
  push_u16(&bytes, nightfall::quickdraw::kPictRGBForeColor);
  push_u16(&bytes, 0u);
  push_u16(&bytes, 0xFFFFu);
  push_u16(&bytes, 0u);
  push_u16(&bytes, nightfall::quickdraw::kPictPaintRect);
  push_rect(&bytes, 2, 2, 4, 4);
  push_u16(&bytes, nightfall::quickdraw::kPictEndPic);

  CHECK(state.draw_picture(bytes.data(), bytes.size(), nightfall::quickdraw::Rect{10, 10, 26, 26}, nullptr) ==
        NF_OK);

  CHECK(state.pixel(0x00002000u, 12, 12) == 0x0000FF00u);
  CHECK(state.pixel(0x00002000u, 13, 13) == 0x0000FF00u);
  CHECK(state.pixel(0x00002000u, 2, 2) == 0u);
  CHECK(state.pixel(0x00002000u, 10, 10) == 0u);
}

TEST_CASE("QuickDraw DrawPicture preserves parser error behavior and requires a current port") {
  nightfall::quickdraw::PortState state;
  std::vector<uint8_t> bytes = playback_picture();

  CHECK(state.draw_picture(bytes.data(), bytes.size(), nightfall::quickdraw::Rect{0, 0, 16, 16}, nullptr) ==
        NF_ERROR_INVALID_ARGUMENT);

  CHECK(state.init_graf(0x00001000u) == NF_OK);
  CHECK(state.open_port(0x00002000u) == NF_OK);
  std::vector<uint8_t> unsupported = minimal_picture();
  push_u16(&unsupported, 0x7777u);
  CHECK(state.draw_picture(unsupported.data(), unsupported.size(), nightfall::quickdraw::Rect{0, 0, 16, 16},
                           nullptr) == NF_ERROR_UNIMPLEMENTED);
}
