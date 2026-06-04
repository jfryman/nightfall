#include "doctest.h"

#include "QuickDrawPort.h"

using nightfall::quickdraw::PortState;
using nightfall::quickdraw::RGBColor;

namespace {

PortState opened_state() {
  PortState state;
  CHECK(state.init_graf(0x00001000u) == NF_OK);
  CHECK(state.open_port(0x00002000u) == NF_OK);
  CHECK(state.rgb_fore_color(RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu}) == NF_OK);
  return state;
}

uint32_t packed_point(int16_t h, int16_t v) {
  return (static_cast<uint32_t>(static_cast<uint16_t>(v)) << 16u) |
         static_cast<uint32_t>(static_cast<uint16_t>(h));
}

}  // namespace

TEST_CASE("QuickDraw MoveTo and Move update pen location without drawing") {
  PortState state = opened_state();

  CHECK(state.move_to(3, 4) == NF_OK);
  const auto *after_move_to = state.find_port(0x00002000u);
  REQUIRE(after_move_to != nullptr);
  CHECK(after_move_to->pn_loc.h == 3);
  CHECK(after_move_to->pn_loc.v == 4);
  CHECK(state.pixel(0x00002000u, 4, 3) == 0u);

  CHECK(state.move(5, -2) == NF_OK);
  const auto *after_move = state.find_port(0x00002000u);
  REQUIRE(after_move != nullptr);
  CHECK(after_move->pn_loc.h == 8);
  CHECK(after_move->pn_loc.v == 2);
  CHECK(state.pixel(0x00002000u, 2, 8) == 0u);
  CHECK(state.trace_word(state.trace_count() - 2u) == nightfall::quickdraw::kTrapMoveTo);
  CHECK(state.trace_word(state.trace_count() - 1u) == nightfall::quickdraw::kTrapMove);
}

TEST_CASE("QuickDraw LineTo draws a horizontal scanline and updates pen location") {
  PortState state = opened_state();
  CHECK(state.move_to(2, 3) == NF_OK);

  CHECK(state.line_to(6, 3) == NF_OK);

  CHECK(state.pixel(0x00002000u, 3, 2) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 3, 3) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 3, 4) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 3, 5) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 3, 6) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 2, 4) == 0u);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->pn_loc.h == 6);
  CHECK(port->pn_loc.v == 3);
}

TEST_CASE("QuickDraw Line draws a relative vertical scanline") {
  PortState state = opened_state();
  CHECK(state.move_to(5, 1) == NF_OK);

  CHECK(state.line(0, 4) == NF_OK);

  CHECK(state.pixel(0x00002000u, 1, 5) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 2, 5) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 3, 5) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 4, 5) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 5, 5) == nightfall::quickdraw::kWhitePixel);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->pn_loc.h == 5);
  CHECK(port->pn_loc.v == 5);
}

TEST_CASE("QuickDraw LineTo draws a deterministic diagonal") {
  PortState state = opened_state();
  CHECK(state.move_to(0, 0) == NF_OK);

  CHECK(state.line_to(3, 3) == NF_OK);

  CHECK(state.pixel(0x00002000u, 0, 0) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 1, 1) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 2, 2) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 3, 3) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 1, 2) == 0u);
}

TEST_CASE("QuickDraw line operations require an open current port") {
  PortState state;
  CHECK(state.move_to(1, 1) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.move(1, 1) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.line_to(1, 1) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.line(1, 1) == NF_ERROR_INVALID_ARGUMENT);
}

TEST_CASE("QuickDraw trap-word dispatcher covers Phase 4.3 line words") {
  PortState state;
  CHECK(state.dispatch(nightfall::quickdraw::kTrapInitGraf, 0x00001000u) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapOpenPort, 0x00002000u) == NF_OK);
  CHECK(state.rgb_fore_color(RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu}) == NF_OK);

  CHECK(state.dispatch(nightfall::quickdraw::kTrapMoveTo, packed_point(1, 1)) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapLineTo, packed_point(4, 1)) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapMove, packed_point(0, 2)) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapLine, packed_point(3, 0)) == NF_OK);

  CHECK(state.pixel(0x00002000u, 1, 1) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 1, 4) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00002000u, 2, 4) == 0u);
  CHECK(state.pixel(0x00002000u, 3, 7) == nightfall::quickdraw::kWhitePixel);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->pn_loc.h == 7);
  CHECK(port->pn_loc.v == 3);
  CHECK(state.trace_word(state.trace_count() - 1u) == nightfall::quickdraw::kTrapLine);
}
