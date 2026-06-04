#include "doctest.h"

#include "QuickDrawPort.h"

using nightfall::quickdraw::PortState;
using nightfall::quickdraw::RGBColor;
using nightfall::quickdraw::Rect;

namespace {

PortState three_port_state() {
  PortState state;
  CHECK(state.init_graf(0x00001000u) == NF_OK);
  CHECK(state.open_port(0x00002000u) == NF_OK);
  CHECK(state.open_port(0x00003000u) == NF_OK);
  CHECK(state.open_port(0x00004000u) == NF_OK);
  return state;
}

void fill_port(PortState &state, uint32_t port, const Rect &rect, const RGBColor &color) {
  CHECK(state.set_port(port) == NF_OK);
  CHECK(state.rgb_fore_color(color) == NF_OK);
  CHECK(state.paint_rect(rect) == NF_OK);
}

}  // namespace

TEST_CASE("QuickDraw CopyBits srcCopy copies modeled 32-bit pixels between ports") {
  PortState state = three_port_state();
  fill_port(state, 0x00002000u, Rect{0, 0, 3, 3}, RGBColor{0xFFFFu, 0x0000u, 0x0000u});

  CHECK(state.copy_bits(0x00002000u, 0x00003000u, Rect{0, 0, 3, 3}, Rect{5, 6, 8, 9},
                        nightfall::quickdraw::kSrcCopy, nightfall::quickdraw::kNilAddress) == NF_OK);

  CHECK(state.pixel(0x00003000u, 5, 6) == 0x00FF0000u);
  CHECK(state.pixel(0x00003000u, 7, 8) == 0x00FF0000u);
  CHECK(state.pixel(0x00003000u, 4, 6) == 0u);

  const auto *destination = state.find_port(0x00003000u);
  REQUIRE(destination != nullptr);
  CHECK(destination->port_pix_map.pixel_size == 32u);
  CHECK(destination->port_pix_map.bounds.right == 640);
}

TEST_CASE("QuickDraw CopyBits applies source modes against destination pixels") {
  PortState state = three_port_state();
  fill_port(state, 0x00002000u, Rect{0, 0, 2, 2}, RGBColor{0xFFFFu, 0x0000u, 0x0000u});
  fill_port(state, 0x00003000u, Rect{0, 0, 2, 2}, RGBColor{0x0000u, 0x0000u, 0xFFFFu});

  CHECK(state.copy_bits(0x00002000u, 0x00003000u, Rect{0, 0, 2, 2}, Rect{0, 0, 2, 2},
                        nightfall::quickdraw::kSrcOr, nightfall::quickdraw::kNilAddress) == NF_OK);
  CHECK(state.pixel(0x00003000u, 0, 0) == 0x00FF00FFu);

  CHECK(state.copy_bits(0x00002000u, 0x00003000u, Rect{0, 0, 1, 1}, Rect{3, 3, 4, 4},
                        nightfall::quickdraw::kSrcXor, nightfall::quickdraw::kNilAddress) == NF_OK);
  CHECK(state.pixel(0x00003000u, 3, 3) == 0x00FF0000u);

  fill_port(state, 0x00003000u, Rect{5, 5, 6, 6}, RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu});
  CHECK(state.copy_bits(0x00002000u, 0x00003000u, Rect{0, 0, 1, 1}, Rect{5, 5, 6, 6},
                        nightfall::quickdraw::kSrcBic, nightfall::quickdraw::kNilAddress) == NF_OK);
  CHECK(state.pixel(0x00003000u, 5, 5) == 0x0000FFFFu);
}

TEST_CASE("QuickDraw CopyBits scales source pixels into a larger destination rectangle") {
  PortState state = three_port_state();
  fill_port(state, 0x00002000u, Rect{0, 0, 1, 1}, RGBColor{0xFFFFu, 0x0000u, 0x0000u});
  fill_port(state, 0x00002000u, Rect{0, 1, 1, 2}, RGBColor{0x0000u, 0xFFFFu, 0x0000u});
  fill_port(state, 0x00002000u, Rect{1, 0, 2, 1}, RGBColor{0x0000u, 0x0000u, 0xFFFFu});
  fill_port(state, 0x00002000u, Rect{1, 1, 2, 2}, RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu});

  CHECK(state.copy_bits(0x00002000u, 0x00003000u, Rect{0, 0, 2, 2}, Rect{0, 0, 4, 4},
                        nightfall::quickdraw::kSrcCopy, nightfall::quickdraw::kNilAddress) == NF_OK);

  CHECK(state.pixel(0x00003000u, 0, 0) == 0x00FF0000u);
  CHECK(state.pixel(0x00003000u, 0, 3) == 0x0000FF00u);
  CHECK(state.pixel(0x00003000u, 3, 0) == 0x000000FFu);
  CHECK(state.pixel(0x00003000u, 3, 3) == 0x00FFFFFFu);
}

TEST_CASE("QuickDraw CopyBits respects mask region and current-port clip") {
  PortState state = three_port_state();
  fill_port(state, 0x00002000u, Rect{0, 0, 6, 6}, RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu});
  CHECK(state.set_port(0x00003000u) == NF_OK);

  uint32_t mask_region = 0u;
  uint32_t clip_region = 0u;
  CHECK(state.new_rgn(&mask_region) == NF_OK);
  CHECK(state.new_rgn(&clip_region) == NF_OK);
  CHECK(state.set_rect_rgn(mask_region, 1, 1, 5, 5) == NF_OK);
  CHECK(state.set_rect_rgn(clip_region, 2, 2, 6, 6) == NF_OK);
  CHECK(state.set_clip(clip_region) == NF_OK);

  CHECK(state.copy_bits(0x00002000u, 0x00003000u, Rect{0, 0, 6, 6}, Rect{0, 0, 6, 6},
                        nightfall::quickdraw::kSrcCopy, mask_region) == NF_OK);

  CHECK(state.pixel(0x00003000u, 1, 1) == 0u);
  CHECK(state.pixel(0x00003000u, 2, 2) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00003000u, 4, 4) == nightfall::quickdraw::kWhitePixel);
  CHECK(state.pixel(0x00003000u, 5, 5) == 0u);
}

TEST_CASE("QuickDraw CopyMask copies only where the modeled mask is active") {
  PortState state = three_port_state();
  fill_port(state, 0x00002000u, Rect{0, 0, 4, 4}, RGBColor{0x0000u, 0xFFFFu, 0x0000u});
  fill_port(state, 0x00004000u, Rect{0, 0, 2, 2}, RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu});

  CHECK(state.copy_mask(0x00002000u, 0x00004000u, 0x00003000u,
                        Rect{0, 0, 4, 4}, Rect{0, 0, 4, 4}, Rect{0, 0, 4, 4}) == NF_OK);

  CHECK(state.pixel(0x00003000u, 0, 0) == 0x0000FF00u);
  CHECK(state.pixel(0x00003000u, 1, 1) == 0x0000FF00u);
  CHECK(state.pixel(0x00003000u, 2, 2) == 0u);
}

TEST_CASE("QuickDraw bit transfer operations reject missing ports or invalid mask geometry") {
  PortState state = three_port_state();
  CHECK(state.copy_bits(0x00002000u, 0x0000F000u, Rect{0, 0, 1, 1}, Rect{0, 0, 1, 1},
                        nightfall::quickdraw::kSrcCopy, nightfall::quickdraw::kNilAddress) ==
        NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.copy_mask(0x00002000u, 0x00004000u, 0x00003000u,
                        Rect{0, 0, 2, 2}, Rect{0, 0, 1, 1}, Rect{0, 0, 2, 2}) ==
        NF_ERROR_INVALID_ARGUMENT);
}
