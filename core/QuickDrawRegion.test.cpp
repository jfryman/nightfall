#include "doctest.h"

#include "QuickDrawPort.h"

using nightfall::quickdraw::Point;
using nightfall::quickdraw::PortState;
using nightfall::quickdraw::Rect;

namespace {

uint32_t new_region(PortState &state) {
  uint32_t region = 0u;
  CHECK(state.new_rgn(&region) == NF_OK);
  CHECK(region != nightfall::quickdraw::kNilAddress);
  return region;
}

PortState opened_state() {
  PortState state;
  CHECK(state.init_graf(0x00001000u) == NF_OK);
  CHECK(state.open_port(0x00002000u) == NF_OK);
  return state;
}

}  // namespace

TEST_CASE("QuickDraw NewRgn creates an empty modeled region and DisposeRgn releases it") {
  PortState state;

  uint32_t region = 0u;
  CHECK(state.new_rgn(&region) == NF_OK);
  const auto *created = state.find_region(region);
  REQUIRE(created != nullptr);
  CHECK(created->allocated == true);
  CHECK(created->bounds.bottom == 0);

  bool empty = false;
  CHECK(state.empty_rgn(region, &empty) == NF_OK);
  CHECK(empty == true);

  CHECK(state.dispose_rgn(region) == NF_OK);
  CHECK(state.find_region(region) == nullptr);
  CHECK(state.dispose_rgn(region) == NF_ERROR_INVALID_ARGUMENT);
}

TEST_CASE("QuickDraw SetRectRgn and RectRgn replace region structure") {
  PortState state;
  const uint32_t region = new_region(state);

  CHECK(state.set_rect_rgn(region, 2, 1, 6, 4) == NF_OK);

  const auto *rect = state.find_region(region);
  REQUIRE(rect != nullptr);
  CHECK(rect->bounds.top == 1);
  CHECK(rect->bounds.left == 2);
  CHECK(rect->bounds.bottom == 4);
  CHECK(rect->bounds.right == 6);

  bool contains = false;
  CHECK(state.pt_in_rgn(Point{1, 2}, region, &contains) == NF_OK);
  CHECK(contains == true);
  CHECK(state.pt_in_rgn(Point{4, 5}, region, &contains) == NF_OK);
  CHECK(contains == false);

  CHECK(state.rect_rgn(region, Rect{3, 3, 3, 7}) == NF_OK);
  bool empty = false;
  CHECK(state.empty_rgn(region, &empty) == NF_OK);
  CHECK(empty == true);
  const auto *emptied = state.find_region(region);
  REQUIRE(emptied != nullptr);
  CHECK(emptied->bounds.right == 0);
}

TEST_CASE("QuickDraw OffsetRgn moves the modeled region without changing its shape") {
  PortState state;
  const uint32_t region = new_region(state);
  CHECK(state.set_rect_rgn(region, 1, 1, 3, 3) == NF_OK);

  CHECK(state.offset_rgn(region, 4, 2) == NF_OK);

  const auto *moved = state.find_region(region);
  REQUIRE(moved != nullptr);
  CHECK(moved->bounds.top == 3);
  CHECK(moved->bounds.left == 5);
  CHECK(moved->bounds.bottom == 5);
  CHECK(moved->bounds.right == 7);

  bool contains = false;
  CHECK(state.pt_in_rgn(Point{3, 5}, region, &contains) == NF_OK);
  CHECK(contains == true);
  CHECK(state.pt_in_rgn(Point{1, 1}, region, &contains) == NF_OK);
  CHECK(contains == false);
}

TEST_CASE("QuickDraw CopyRgn and EqualRgn duplicate independent region structure") {
  PortState state;
  const uint32_t source = new_region(state);
  const uint32_t destination = new_region(state);
  CHECK(state.set_rect_rgn(source, 1, 1, 4, 4) == NF_OK);

  CHECK(state.copy_rgn(source, destination) == NF_OK);

  bool equal = false;
  CHECK(state.equal_rgn(source, destination, &equal) == NF_OK);
  CHECK(equal == true);

  CHECK(state.offset_rgn(source, 1, 0) == NF_OK);
  CHECK(state.equal_rgn(source, destination, &equal) == NF_OK);
  CHECK(equal == false);
}

TEST_CASE("QuickDraw UnionRgn, SectRgn, and DiffRgn update destination structure") {
  PortState state;
  const uint32_t a = new_region(state);
  const uint32_t b = new_region(state);
  const uint32_t destination = new_region(state);
  CHECK(state.set_rect_rgn(a, 1, 1, 5, 5) == NF_OK);
  CHECK(state.set_rect_rgn(b, 3, 3, 7, 7) == NF_OK);

  bool contains = false;
  CHECK(state.union_rgn(a, b, destination) == NF_OK);
  CHECK(state.pt_in_rgn(Point{1, 1}, destination, &contains) == NF_OK);
  CHECK(contains == true);
  CHECK(state.pt_in_rgn(Point{6, 6}, destination, &contains) == NF_OK);
  CHECK(contains == true);

  CHECK(state.sect_rgn(a, b, destination) == NF_OK);
  CHECK(state.pt_in_rgn(Point{2, 2}, destination, &contains) == NF_OK);
  CHECK(contains == false);
  CHECK(state.pt_in_rgn(Point{3, 3}, destination, &contains) == NF_OK);
  CHECK(contains == true);
  const auto *intersection = state.find_region(destination);
  REQUIRE(intersection != nullptr);
  CHECK(intersection->bounds.top == 3);
  CHECK(intersection->bounds.left == 3);
  CHECK(intersection->bounds.bottom == 5);
  CHECK(intersection->bounds.right == 5);

  CHECK(state.diff_rgn(a, b, destination) == NF_OK);
  CHECK(state.pt_in_rgn(Point{1, 1}, destination, &contains) == NF_OK);
  CHECK(contains == true);
  CHECK(state.pt_in_rgn(Point{3, 3}, destination, &contains) == NF_OK);
  CHECK(contains == false);
}

TEST_CASE("QuickDraw SetClip and GetClip copy region structure through the current port") {
  PortState state = opened_state();
  const uint32_t source = new_region(state);
  const uint32_t saved = new_region(state);
  CHECK(state.set_rect_rgn(source, 2, 2, 8, 8) == NF_OK);

  CHECK(state.set_clip(source) == NF_OK);
  CHECK(state.offset_rgn(source, 4, 0) == NF_OK);
  CHECK(state.get_clip(saved) == NF_OK);

  bool contains = false;
  CHECK(state.pt_in_rgn(Point{2, 2}, saved, &contains) == NF_OK);
  CHECK(contains == true);
  CHECK(state.pt_in_rgn(Point{2, 10}, saved, &contains) == NF_OK);
  CHECK(contains == false);

  const auto *port = state.find_port(0x00002000u);
  REQUIRE(port != nullptr);
  CHECK(port->clip_rgn.bounds.top == 2);
  CHECK(port->clip_rgn.bounds.right == 8);
}

TEST_CASE("QuickDraw region operations reject missing handles") {
  PortState state;
  bool value = false;
  CHECK(state.dispose_rgn(0x00010000u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.set_rect_rgn(0x00010000u, 0, 0, 1, 1) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.copy_rgn(0x00010000u, 0x00010004u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.union_rgn(0x00010000u, 0x00010004u, 0x00010008u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.empty_rgn(0x00010000u, &value) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.equal_rgn(0x00010000u, 0x00010004u, &value) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.pt_in_rgn(Point{0, 0}, 0x00010000u, &value) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.set_clip(0x00010000u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.get_clip(0x00010000u) == NF_ERROR_INVALID_ARGUMENT);
}
