#include "QuickDrawPort.h"

namespace nightfall::quickdraw {

#define NF_TOOLBOX_TRAP(trap_word) record_trace(trap_word)

namespace {

bool empty_rect(const Rect &rect) {
  return rect.right <= rect.left || rect.bottom <= rect.top;
}

void clear_region(Region &region) {
  region.bounds = Rect{0, 0, 0, 0};
  for (uint64_t &row : region.rows) {
    row = 0u;
  }
}

bool region_empty(const Region &region) {
  for (uint64_t row : region.rows) {
    if (row != 0u) {
      return false;
    }
  }
  return true;
}

void recompute_bounds(Region &region) {
  int16_t top = 0;
  int16_t left = 0;
  int16_t bottom = 0;
  int16_t right = 0;
  bool found = false;

  for (int16_t v = 0; v < static_cast<int16_t>(kMaxModeledBitmapHeight); ++v) {
    uint64_t row = region.rows[static_cast<size_t>(v)];
    if (row == 0u) {
      continue;
    }
    for (int16_t h = 0; h < static_cast<int16_t>(kMaxModeledBitmapWidth); ++h) {
      if ((row & (uint64_t{1} << static_cast<uint16_t>(h))) == 0u) {
        continue;
      }
      if (!found) {
        top = v;
        left = h;
        bottom = static_cast<int16_t>(v + 1);
        right = static_cast<int16_t>(h + 1);
        found = true;
      } else {
        if (v < top) {
          top = v;
        }
        if (h < left) {
          left = h;
        }
        if (v + 1 > bottom) {
          bottom = static_cast<int16_t>(v + 1);
        }
        if (h + 1 > right) {
          right = static_cast<int16_t>(h + 1);
        }
      }
    }
  }

  region.bounds = found ? Rect{top, left, bottom, right} : Rect{0, 0, 0, 0};
}

void set_rect_region(Region &region, const Rect &rect) {
  clear_region(region);
  if (empty_rect(rect)) {
    return;
  }

  for (int16_t v = rect.top; v < rect.bottom; ++v) {
    if (v < 0 || v >= static_cast<int16_t>(kMaxModeledBitmapHeight)) {
      continue;
    }
    for (int16_t h = rect.left; h < rect.right; ++h) {
      if (h < 0 || h >= static_cast<int16_t>(kMaxModeledBitmapWidth)) {
        continue;
      }
      region.rows[static_cast<size_t>(v)] |= uint64_t{1} << static_cast<uint16_t>(h);
    }
  }
  recompute_bounds(region);
}

void copy_region_shape(const Region &source, Region &destination) {
  const uint32_t address = destination.address;
  const bool allocated = destination.allocated;
  destination = source;
  destination.address = address;
  destination.allocated = allocated;
}

bool region_contains(const Region &region, Point point) {
  if (point.v < 0 || point.h < 0 ||
      point.v >= static_cast<int16_t>(kMaxModeledBitmapHeight) ||
      point.h >= static_cast<int16_t>(kMaxModeledBitmapWidth)) {
    return false;
  }
  return (region.rows[static_cast<size_t>(point.v)] & (uint64_t{1} << static_cast<uint16_t>(point.h))) != 0u;
}

}  // namespace

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::new_rgn(uint32_t *out_region_address) {
  NF_TOOLBOX_TRAP(kTrapNewRgn);

  if (out_region_address == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (Region &region : regions_) {
    if (!region.allocated) {
      region = Region{};
      region.allocated = true;
      region.address = next_region_address_;
      next_region_address_ += 4u;
      clear_region(region);
      *out_region_address = region.address;
      return NF_OK;
    }
  }
  return NF_ERROR_RESOURCE_EXHAUSTED;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::dispose_rgn(uint32_t region_address) {
  NF_TOOLBOX_TRAP(kTrapDisposeRgn);

  Region *region = find_region_mutable(region_address);
  if (region == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *region = Region{};
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::set_rect_rgn(uint32_t region_address,
                                  int16_t left,
                                  int16_t top,
                                  int16_t right,
                                  int16_t bottom) {
  NF_TOOLBOX_TRAP(kTrapSetRectRgn);

  Region *region = find_region_mutable(region_address);
  if (region == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  set_rect_region(*region, Rect{top, left, bottom, right});
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::rect_rgn(uint32_t region_address, const Rect &rect) {
  NF_TOOLBOX_TRAP(kTrapRectRgn);

  Region *region = find_region_mutable(region_address);
  if (region == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  set_rect_region(*region, rect);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::offset_rgn(uint32_t region_address, int16_t dh, int16_t dv) {
  NF_TOOLBOX_TRAP(kTrapOffsetRgn);

  Region *region = find_region_mutable(region_address);
  if (region == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  uint64_t shifted[kMaxModeledBitmapHeight]{};
  for (int16_t v = 0; v < static_cast<int16_t>(kMaxModeledBitmapHeight); ++v) {
    for (int16_t h = 0; h < static_cast<int16_t>(kMaxModeledBitmapWidth); ++h) {
      if (!region_contains(*region, Point{v, h})) {
        continue;
      }
      const int16_t shifted_v = static_cast<int16_t>(v + dv);
      const int16_t shifted_h = static_cast<int16_t>(h + dh);
      if (shifted_v < 0 || shifted_h < 0 ||
          shifted_v >= static_cast<int16_t>(kMaxModeledBitmapHeight) ||
          shifted_h >= static_cast<int16_t>(kMaxModeledBitmapWidth)) {
        continue;
      }
      shifted[static_cast<size_t>(shifted_v)] |= uint64_t{1} << static_cast<uint16_t>(shifted_h);
    }
  }
  for (size_t row = 0; row < kMaxModeledBitmapHeight; ++row) {
    region->rows[row] = shifted[row];
  }
  recompute_bounds(*region);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::copy_rgn(uint32_t source_region_address, uint32_t destination_region_address) {
  NF_TOOLBOX_TRAP(kTrapCopyRgn);

  const Region *source = find_region(source_region_address);
  Region *destination = find_region_mutable(destination_region_address);
  if (source == nullptr || destination == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  copy_region_shape(*source, *destination);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::union_rgn(uint32_t source_a_address, uint32_t source_b_address, uint32_t destination_address) {
  NF_TOOLBOX_TRAP(kTrapUnionRgn);

  const Region *source_a = find_region(source_a_address);
  const Region *source_b = find_region(source_b_address);
  Region *destination = find_region_mutable(destination_address);
  if (source_a == nullptr || source_b == nullptr || destination == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (size_t row = 0; row < kMaxModeledBitmapHeight; ++row) {
    destination->rows[row] = source_a->rows[row] | source_b->rows[row];
  }
  recompute_bounds(*destination);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::sect_rgn(uint32_t source_a_address, uint32_t source_b_address, uint32_t destination_address) {
  NF_TOOLBOX_TRAP(kTrapSectRgn);

  const Region *source_a = find_region(source_a_address);
  const Region *source_b = find_region(source_b_address);
  Region *destination = find_region_mutable(destination_address);
  if (source_a == nullptr || source_b == nullptr || destination == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (size_t row = 0; row < kMaxModeledBitmapHeight; ++row) {
    destination->rows[row] = source_a->rows[row] & source_b->rows[row];
  }
  recompute_bounds(*destination);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::diff_rgn(uint32_t source_a_address, uint32_t source_b_address, uint32_t destination_address) {
  NF_TOOLBOX_TRAP(kTrapDiffRgn);

  const Region *source_a = find_region(source_a_address);
  const Region *source_b = find_region(source_b_address);
  Region *destination = find_region_mutable(destination_address);
  if (source_a == nullptr || source_b == nullptr || destination == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (size_t row = 0; row < kMaxModeledBitmapHeight; ++row) {
    destination->rows[row] = source_a->rows[row] & ~source_b->rows[row];
  }
  recompute_bounds(*destination);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::empty_rgn(uint32_t region_address, bool *out_empty) const {
  NF_TOOLBOX_TRAP(kTrapEmptyRgn);

  const Region *region = find_region(region_address);
  if (region == nullptr || out_empty == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_empty = region_empty(*region);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::equal_rgn(uint32_t region_a_address, uint32_t region_b_address, bool *out_equal) const {
  NF_TOOLBOX_TRAP(kTrapEqualRgn);

  const Region *region_a = find_region(region_a_address);
  const Region *region_b = find_region(region_b_address);
  if (region_a == nullptr || region_b == nullptr || out_equal == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  bool equal = true;
  for (size_t row = 0; row < kMaxModeledBitmapHeight; ++row) {
    if (region_a->rows[row] != region_b->rows[row]) {
      equal = false;
      break;
    }
  }
  *out_equal = equal;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::pt_in_rgn(Point point, uint32_t region_address, bool *out_contains) const {
  NF_TOOLBOX_TRAP(kTrapPtInRgn);

  const Region *region = find_region(region_address);
  if (region == nullptr || out_contains == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_contains = region_contains(*region, point);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::set_clip(uint32_t region_address) {
  NF_TOOLBOX_TRAP(kTrapSetClip);

  const Region *source = find_region(region_address);
  GrafPort *port = find_port_mutable(globals_.the_port);
  if (source == nullptr || port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  copy_region_shape(*source, port->clip_rgn);
  port->clip_rgn.address = kNilAddress;
  port->clip_rgn.allocated = true;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Region Operations".
nf_status PortState::get_clip(uint32_t region_address) {
  NF_TOOLBOX_TRAP(kTrapGetClip);

  Region *destination = find_region_mutable(region_address);
  const GrafPort *port = find_port(globals_.the_port);
  if (destination == nullptr || port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  copy_region_shape(port->clip_rgn, *destination);
  destination->allocated = true;
  return NF_OK;
}

#undef NF_TOOLBOX_TRAP

}  // namespace nightfall::quickdraw
