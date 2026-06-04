#include "doctest.h"

#include "QuickDrawPort.h"

#include <cstring>

using nightfall::quickdraw::BitMap;
using nightfall::quickdraw::PortState;
using nightfall::quickdraw::Rect;

namespace {

bool pattern_is(const nightfall::quickdraw::Pattern &pattern, uint8_t value) {
  for (uint8_t byte : pattern.bytes) {
    if (byte != value) {
      return false;
    }
  }
  return true;
}

}  // namespace

TEST_CASE("QuickDraw InitGraf initializes modeled globals before port use") {
  PortState state;

  CHECK(state.globals().initialized == false);
  CHECK(state.init_graf(0x00001000u) == NF_OK);

  const auto &globals = state.globals();
  CHECK(globals.initialized == true);
  CHECK(globals.global_ptr == 0x00001000u);
  CHECK(globals.the_port == nightfall::quickdraw::kNilAddress);
  CHECK(pattern_is(globals.white, 0x00u));
  CHECK(pattern_is(globals.black, 0xFFu));
  CHECK(globals.screen_bits.row_bytes == nightfall::quickdraw::kDefaultScreenRowBytes);
  CHECK(globals.screen_bits.bounds.bottom == 480);
  CHECK(globals.screen_bits.bounds.right == 640);
  CHECK(globals.rand_seed == 1);
  REQUIRE(state.trace_count() == 1u);
  CHECK(state.trace_word(0u) == nightfall::quickdraw::kTrapInitGraf);
}

TEST_CASE("QuickDraw OpenPort initializes port state and makes it current") {
  PortState state;
  REQUIRE(state.init_graf(0x00001000u) == NF_OK);

  CHECK(state.open_port(0x00002000u) == NF_OK);

  uint32_t current_port = 0u;
  CHECK(state.get_port(&current_port) == NF_OK);
  CHECK(current_port == 0x00002000u);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->open == true);
  CHECK(port->device == 0);
  CHECK(port->port_bits.row_bytes == nightfall::quickdraw::kDefaultScreenRowBytes);
  CHECK(port->port_rect.bottom == 480);
  CHECK(port->port_rect.right == 640);
  CHECK(port->vis_rgn.allocated == true);
  CHECK(port->vis_rgn.bounds.bottom == 480);
  CHECK(port->clip_rgn.allocated == true);
  CHECK(port->clip_rgn.bounds.top == -32768);
  CHECK(port->clip_rgn.bounds.right == 32767);
  CHECK(port->pn_loc.v == 0);
  CHECK(port->pn_loc.h == 0);
  CHECK(port->pn_size.v == 1);
  CHECK(port->pn_size.h == 1);
  CHECK(port->pn_mode == nightfall::quickdraw::kPatCopy);
  CHECK(port->pn_vis == 0);
  CHECK(port->tx_mode == nightfall::quickdraw::kSrcOr);
  CHECK(port->fg_color == nightfall::quickdraw::kBlackColor);
  CHECK(port->bk_color == nightfall::quickdraw::kWhiteColor);
  CHECK(port->pic_save == nightfall::quickdraw::kNilAddress);
  CHECK(port->graf_procs == nightfall::quickdraw::kNilAddress);

  REQUIRE(state.trace_count() == 4u);
  CHECK(state.trace_word(1u) == nightfall::quickdraw::kTrapOpenPort);
  CHECK(state.trace_word(2u) == nightfall::quickdraw::kTrapSetPort);
  CHECK(state.trace_word(3u) == nightfall::quickdraw::kTrapGetPort);
}

TEST_CASE("QuickDraw SetPort and GetPort preserve independent port state") {
  PortState state;
  REQUIRE(state.init_graf(0x00001000u) == NF_OK);
  REQUIRE(state.open_port(0x00002000u) == NF_OK);
  REQUIRE(state.open_port(0x00003000u) == NF_OK);

  uint32_t current_port = 0u;
  CHECK(state.get_port(&current_port) == NF_OK);
  CHECK(current_port == 0x00003000u);

  CHECK(state.set_port(0x00002000u) == NF_OK);
  CHECK(state.get_port(&current_port) == NF_OK);
  CHECK(current_port == 0x00002000u);

  const auto *first_port = state.find_port(0x00002000u);
  const auto *second_port = state.find_port(0x00003000u);
  REQUIRE(first_port != nullptr);
  REQUIRE(second_port != nullptr);
  CHECK(first_port->pn_size.v == 1);
  CHECK(second_port->pn_size.v == 1);
  CHECK(first_port->vis_rgn.allocated == true);
  CHECK(second_port->vis_rgn.allocated == true);

  CHECK(state.get_port(nullptr) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.set_port(0x00004000u) == NF_ERROR_INVALID_ARGUMENT);
}

TEST_CASE("QuickDraw SetPortBits replaces only the current port bitmap") {
  PortState state;
  REQUIRE(state.init_graf(0x00001000u) == NF_OK);
  REQUIRE(state.open_port(0x00002000u) == NF_OK);
  REQUIRE(state.open_port(0x00003000u) == NF_OK);
  REQUIRE(state.set_port(0x00002000u) == NF_OK);

  const BitMap custom_bitmap{
      0x00004000u,
      128u,
      Rect{10, 20, 30, 60},
  };
  CHECK(state.set_port_bits(custom_bitmap) == NF_OK);

  const auto *first_port = state.find_port(0x00002000u);
  const auto *second_port = state.find_port(0x00003000u);
  REQUIRE(first_port != nullptr);
  REQUIRE(second_port != nullptr);
  CHECK(first_port->port_bits.base_addr == 0x00004000u);
  CHECK(first_port->port_bits.row_bytes == 128u);
  CHECK(first_port->port_bits.bounds.top == 10);
  CHECK(first_port->port_bits.bounds.right == 60);
  CHECK(first_port->port_rect.right == 640);
  CHECK(second_port->port_bits.base_addr == 0u);
  CHECK(second_port->port_bits.bounds.right == 640);
}

TEST_CASE("QuickDraw ClosePort releases modeled regions without changing the current port global") {
  PortState state;
  REQUIRE(state.init_graf(0x00001000u) == NF_OK);
  REQUIRE(state.open_port(0x00002000u) == NF_OK);

  CHECK(state.close_port(0x00002000u) == NF_OK);
  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->open == false);
  CHECK(port->vis_rgn.allocated == false);
  CHECK(port->clip_rgn.allocated == false);

  uint32_t current_port = 0u;
  CHECK(state.get_port(&current_port) == NF_OK);
  CHECK(current_port == 0x00002000u);
  CHECK(state.close_port(0x00002000u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.set_port_bits(BitMap{}) == NF_ERROR_INVALID_ARGUMENT);
}

TEST_CASE("QuickDraw trap-word dispatcher covers Phase 4.1 words") {
  PortState state;

  CHECK(state.dispatch(nightfall::quickdraw::kTrapInitGraf, 0x00001000u) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapOpenPort, 0x00002000u) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapSetPort, 0x00002000u) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapGetPort, 0x0000F000u) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapSetPortBits, 0x00004000u) == NF_OK);
  CHECK(state.dispatch(nightfall::quickdraw::kTrapClosePort, 0x00002000u) == NF_OK);
  CHECK(state.dispatch(0xA8A1u, 0x00002000u) == NF_ERROR_UNIMPLEMENTED);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->port_bits.base_addr == 0x00004000u);
  CHECK(port->open == false);
}
