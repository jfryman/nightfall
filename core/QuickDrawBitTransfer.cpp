#include "QuickDrawPort.h"

namespace nightfall::quickdraw {

#define NF_TOOLBOX_TRAP(trap_word) record_trace(trap_word)

namespace {

bool empty_rect(const Rect &rect) {
  return rect.right <= rect.left || rect.bottom <= rect.top;
}

bool in_modeled_bounds(int16_t h, int16_t v) {
  return h >= 0 && v >= 0 &&
         h < static_cast<int16_t>(kMaxModeledBitmapWidth) &&
         v < static_cast<int16_t>(kMaxModeledBitmapHeight);
}

uint32_t pixel_at(const GrafPort &port, int16_t h, int16_t v) {
  if (!in_modeled_bounds(h, v)) {
    return 0u;
  }
  return port.pixels[(static_cast<size_t>(v) * kMaxModeledBitmapWidth) + static_cast<size_t>(h)];
}

void set_pixel(GrafPort &port, int16_t h, int16_t v, uint32_t pixel) {
  if (!in_modeled_bounds(h, v)) {
    return;
  }
  port.pixels[(static_cast<size_t>(v) * kMaxModeledBitmapWidth) + static_cast<size_t>(h)] = pixel & kWhitePixel;
}

bool region_contains_point(const Region &region, Point point) {
  if (!in_modeled_bounds(point.h, point.v)) {
    return false;
  }
  return (region.rows[static_cast<size_t>(point.v)] & (uint64_t{1} << static_cast<uint16_t>(point.h))) != 0u;
}

bool clip_allows(const GrafPort &port, Point point) {
  if (port.clip_rgn.bounds.top == -32768 && port.clip_rgn.bounds.left == -32768 &&
      port.clip_rgn.bounds.bottom == 32767 && port.clip_rgn.bounds.right == 32767) {
    return true;
  }
  return region_contains_point(port.clip_rgn, point);
}

int16_t scaled_coordinate(int16_t destination_value, int16_t destination_min, int16_t destination_max,
                          int16_t source_min, int16_t source_max) {
  const int destination_span = destination_max - destination_min;
  const int source_span = source_max - source_min;
  if (destination_span <= 0 || source_span <= 0) {
    return source_min;
  }
  const int offset = destination_value - destination_min;
  return static_cast<int16_t>(source_min + ((offset * source_span) / destination_span));
}

uint32_t apply_source_mode(uint32_t source, uint32_t destination, int16_t mode) {
  switch (mode & 0x0007) {
    case kSrcCopy:
      return source;
    case kSrcOr:
      return source | destination;
    case kSrcXor:
      return source ^ destination;
    case kSrcBic:
      return destination & (~source & kWhitePixel);
    case kNotSrcCopy:
      return ~source & kWhitePixel;
    case kNotSrcOr:
      return (~source & kWhitePixel) | destination;
    case kNotSrcXor:
      return (~source & kWhitePixel) ^ destination;
    case kNotSrcBic:
      return destination & source;
    default:
      return source;
  }
}

}  // namespace

// Source: docs/clean-room-sources.md, "QuickDraw Bit Transfer".
nf_status PortState::copy_bits(uint32_t source_port_address,
                               uint32_t destination_port_address,
                               const Rect &source_rect,
                               const Rect &destination_rect,
                               int16_t mode,
                               uint32_t mask_region_address) {
  NF_TOOLBOX_TRAP(kTrapCopyBits);

  const GrafPort *source = find_port(source_port_address);
  GrafPort *destination = find_port_mutable(destination_port_address);
  const Region *mask = mask_region_address == kNilAddress ? nullptr : find_region(mask_region_address);
  if (source == nullptr || destination == nullptr || !source->open || !destination->open ||
      (mask_region_address != kNilAddress && mask == nullptr)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }
  if (empty_rect(source_rect) || empty_rect(destination_rect)) {
    return NF_OK;
  }

  for (int16_t v = destination_rect.top; v < destination_rect.bottom; ++v) {
    for (int16_t h = destination_rect.left; h < destination_rect.right; ++h) {
      if (!in_modeled_bounds(h, v) || (mask != nullptr && !region_contains_point(*mask, Point{v, h})) ||
          (destination_port_address == globals_.the_port && !clip_allows(*destination, Point{v, h}))) {
        continue;
      }
      const int16_t source_h =
          scaled_coordinate(h, destination_rect.left, destination_rect.right, source_rect.left, source_rect.right);
      const int16_t source_v =
          scaled_coordinate(v, destination_rect.top, destination_rect.bottom, source_rect.top, source_rect.bottom);
      const uint32_t source_pixel = pixel_at(*source, source_h, source_v);
      const uint32_t destination_pixel = pixel_at(*destination, h, v);
      set_pixel(*destination, h, v, apply_source_mode(source_pixel, destination_pixel, mode));
    }
  }
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Bit Transfer".
nf_status PortState::copy_mask(uint32_t source_port_address,
                               uint32_t mask_port_address,
                               uint32_t destination_port_address,
                               const Rect &source_rect,
                               const Rect &mask_rect,
                               const Rect &destination_rect) {
  NF_TOOLBOX_TRAP(kTrapCopyMask);

  const GrafPort *source = find_port(source_port_address);
  const GrafPort *mask = find_port(mask_port_address);
  GrafPort *destination = find_port_mutable(destination_port_address);
  if (source == nullptr || mask == nullptr || destination == nullptr ||
      !source->open || !mask->open || !destination->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }
  if (empty_rect(source_rect) || empty_rect(mask_rect) || empty_rect(destination_rect)) {
    return NF_OK;
  }
  if ((source_rect.right - source_rect.left) != (mask_rect.right - mask_rect.left) ||
      (source_rect.bottom - source_rect.top) != (mask_rect.bottom - mask_rect.top)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (int16_t v = destination_rect.top; v < destination_rect.bottom; ++v) {
    for (int16_t h = destination_rect.left; h < destination_rect.right; ++h) {
      if (!in_modeled_bounds(h, v) ||
          (destination_port_address == globals_.the_port && !clip_allows(*destination, Point{v, h}))) {
        continue;
      }
      const int16_t source_h =
          scaled_coordinate(h, destination_rect.left, destination_rect.right, source_rect.left, source_rect.right);
      const int16_t source_v =
          scaled_coordinate(v, destination_rect.top, destination_rect.bottom, source_rect.top, source_rect.bottom);
      const int16_t mask_h = static_cast<int16_t>(mask_rect.left + (source_h - source_rect.left));
      const int16_t mask_v = static_cast<int16_t>(mask_rect.top + (source_v - source_rect.top));
      if (pixel_at(*mask, mask_h, mask_v) == 0u) {
        continue;
      }
      set_pixel(*destination, h, v, pixel_at(*source, source_h, source_v));
    }
  }
  return NF_OK;
}

#undef NF_TOOLBOX_TRAP

}  // namespace nightfall::quickdraw
