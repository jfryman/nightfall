#include "doctest.h"

#include "QuickDrawPort.h"

using nightfall::quickdraw::Pattern;
using nightfall::quickdraw::PortState;
using nightfall::quickdraw::RGBColor;
using nightfall::quickdraw::Rect;

namespace {

PortState opened_state(uint32_t port_address = 0x00002000u) {
  PortState state;
  CHECK(state.init_graf(0x00001000u) == NF_OK);
  CHECK(state.open_port(port_address) == NF_OK);
  return state;
}

}  // namespace

TEST_CASE("QuickDraw ForeColor and BackColor update the current port colors") {
  PortState state = opened_state();

  CHECK(state.fore_color(nightfall::quickdraw::kRedColor) == NF_OK);
  CHECK(state.back_color(nightfall::quickdraw::kBlueColor) == NF_OK);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->fg_color == nightfall::quickdraw::kRedColor);
  CHECK(port->bk_color == nightfall::quickdraw::kBlueColor);
  CHECK(port->rgb_fg_color.red == 0xFF00u);
  CHECK(port->rgb_fg_color.green == 0x0000u);
  CHECK(port->rgb_bk_color.blue == 0xFF00u);
  CHECK(state.trace_word(state.trace_count() - 2u) == nightfall::quickdraw::kTrapForeColor);
  CHECK(state.trace_word(state.trace_count() - 1u) == nightfall::quickdraw::kTrapBackColor);
}

TEST_CASE("QuickDraw RGBForeColor and RGBBackColor store direct 32-bit modeled pixels") {
  PortState state = opened_state();

  CHECK(state.rgb_fore_color(RGBColor{0x1234u, 0xABCDu, 0x00FFu}) == NF_OK);
  CHECK(state.rgb_back_color(RGBColor{0xFFFFu, 0x8000u, 0x0000u}) == NF_OK);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->rgb_fg_color.red == 0x1234u);
  CHECK(port->rgb_fg_color.green == 0xABCDu);
  CHECK(static_cast<uint32_t>(port->fg_color) == 0x0012AB00u);
  CHECK(static_cast<uint32_t>(port->bk_color) == 0x00FF8000u);
}

TEST_CASE("QuickDraw PaintRect fills with the current foreground pattern and preserves pen location") {
  PortState state = opened_state();
  CHECK(state.rgb_fore_color(RGBColor{0xFFFFu, 0x0000u, 0x0000u}) == NF_OK);

  const auto *before = state.find_port(0x00002000u);
  REQUIRE(before != nullptr);
  const auto pen_before = before->pn_loc;

  CHECK(state.paint_rect(Rect{1, 2, 4, 6}) == NF_OK);

  CHECK(state.pixel(0x00002000u, 1, 2) == 0x00FF0000u);
  CHECK(state.pixel(0x00002000u, 3, 5) == 0x00FF0000u);
  CHECK(state.pixel(0x00002000u, 0, 2) == 0x00000000u);
  CHECK(state.pixel(0x00002000u, 4, 5) == 0x00000000u);

  const auto *after = state.find_port(0x00002000u);
  REQUIRE(after != nullptr);
  CHECK(after->pn_loc.v == pen_before.v);
  CHECK(after->pn_loc.h == pen_before.h);
}

TEST_CASE("QuickDraw EraseRect fills with the current background color") {
  PortState state = opened_state();
  CHECK(state.rgb_fore_color(RGBColor{0xFFFFu, 0x0000u, 0x0000u}) == NF_OK);
  CHECK(state.paint_rect(Rect{0, 0, 5, 5}) == NF_OK);
  CHECK(state.rgb_back_color(RGBColor{0x0000u, 0x0000u, 0xFFFFu}) == NF_OK);

  CHECK(state.erase_rect(Rect{1, 1, 4, 4}) == NF_OK);

  CHECK(state.pixel(0x00002000u, 0, 0) == 0x00FF0000u);
  CHECK(state.pixel(0x00002000u, 1, 1) == 0x000000FFu);
  CHECK(state.pixel(0x00002000u, 3, 3) == 0x000000FFu);
  CHECK(state.pixel(0x00002000u, 4, 4) == 0x00FF0000u);
}

TEST_CASE("QuickDraw FillRect applies the caller pattern with patCopy semantics") {
  PortState state = opened_state();
  CHECK(state.rgb_fore_color(RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu}) == NF_OK);
  CHECK(state.rgb_back_color(RGBColor{0x0000u, 0x0000u, 0x0000u}) == NF_OK);

  const Pattern checker{{0xAAu, 0x55u, 0xAAu, 0x55u, 0xAAu, 0x55u, 0xAAu, 0x55u}};
  CHECK(state.fill_rect(Rect{0, 0, 2, 4}, checker) == NF_OK);

  CHECK(state.pixel(0x00002000u, 0, 0) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 0, 1) == nightfall::quickdraw::kBlackPixel);
  CHECK(state.pixel(0x00002000u, 1, 0) == nightfall::quickdraw::kBlackPixel);
  CHECK(state.pixel(0x00002000u, 1, 1) == nightfall::quickdraw::kWhitePixel);
}

TEST_CASE("QuickDraw FrameRect outlines without painting the interior") {
  PortState state = opened_state();
  CHECK(state.rgb_fore_color(RGBColor{0x0000u, 0xFFFFu, 0x0000u}) == NF_OK);

  CHECK(state.frame_rect(Rect{1, 1, 5, 6}) == NF_OK);

  CHECK(state.pixel(0x00002000u, 1, 1) == 0x0000FF00u);
  CHECK(state.pixel(0x00002000u, 1, 5) == 0x0000FF00u);
  CHECK(state.pixel(0x00002000u, 4, 1) == 0x0000FF00u);
  CHECK(state.pixel(0x00002000u, 4, 5) == 0x0000FF00u);
  CHECK(state.pixel(0x00002000u, 2, 2) == 0x00000000u);
}

TEST_CASE("QuickDraw InvertRect reverses modeled direct pixels") {
  PortState state = opened_state();
  CHECK(state.rgb_fore_color(RGBColor{0x0000u, 0x0000u, 0xFFFFu}) == NF_OK);
  CHECK(state.paint_rect(Rect{0, 0, 3, 3}) == NF_OK);

  CHECK(state.invert_rect(Rect{1, 1, 3, 3}) == NF_OK);

  CHECK(state.pixel(0x00002000u, 0, 0) == 0x000000FFu);
  CHECK(state.pixel(0x00002000u, 1, 1) == 0x00FFFF00u);
  CHECK(state.pixel(0x00002000u, 2, 2) == 0x00FFFF00u);
}

TEST_CASE("QuickDraw rectangle operations require an open current port") {
  PortState state;
  CHECK(state.paint_rect(Rect{0, 0, 1, 1}) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.erase_rect(Rect{0, 0, 1, 1}) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.invert_rect(Rect{0, 0, 1, 1}) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.frame_rect(Rect{0, 0, 1, 1}) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.fill_rect(Rect{0, 0, 1, 1}, Pattern{}) == NF_ERROR_INVALID_ARGUMENT);
}
