#include "QuickDrawPort.h"

namespace nightfall::quickdraw {

#define NF_TOOLBOX_TRAP(trap_word) record_trace(trap_word)

namespace {

uint32_t pixel_for_basic_color(int32_t color) {
  switch (color) {
    case kWhiteColor:
      return kWhitePixel;
    case kBlackColor:
      return kBlackPixel;
    case kRedColor:
      return 0x00FF0000u;
    case kGreenColor:
      return 0x0000FF00u;
    case kBlueColor:
      return 0x000000FFu;
    default:
      return color == kWhiteColor ? kWhitePixel : kBlackPixel;
  }
}

uint32_t pixel_for_rgb(const RGBColor &color) {
  return (static_cast<uint32_t>(color.red >> 8u) << 16u) |
         (static_cast<uint32_t>(color.green >> 8u) << 8u) |
         static_cast<uint32_t>(color.blue >> 8u);
}

bool empty_rect(const Rect &rect) {
  return rect.bottom <= rect.top || rect.right <= rect.left;
}

int16_t clamp_low(int16_t value) {
  return value < 0 ? 0 : value;
}

int16_t clamp_high(int16_t value, int16_t limit) {
  if (value < 0) {
    return 0;
  }
  return value > limit ? limit : value;
}

uint32_t pattern_pixel(const Pattern &pattern, int16_t v, int16_t h, uint32_t foreground, uint32_t background) {
  const auto row = static_cast<size_t>(v) & 7u;
  const auto bit = static_cast<uint8_t>(0x80u >> (static_cast<size_t>(h) & 7u));
  return (pattern.bytes[row] & bit) != 0u ? foreground : background;
}

}  // namespace

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::fore_color(int32_t color) {
  NF_TOOLBOX_TRAP(kTrapForeColor);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->fg_color = color;
  port->fg_pixel = pixel_for_basic_color(color);
  port->rgb_fg_color = RGBColor{
      static_cast<uint16_t>((pixel_for_basic_color(color) >> 8u) & 0xFF00u),
      static_cast<uint16_t>(pixel_for_basic_color(color) & 0xFF00u),
      static_cast<uint16_t>((pixel_for_basic_color(color) << 8u) & 0xFF00u),
  };
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::back_color(int32_t color) {
  NF_TOOLBOX_TRAP(kTrapBackColor);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->bk_color = color;
  port->bk_pixel = pixel_for_basic_color(color);
  port->rgb_bk_color = RGBColor{
      static_cast<uint16_t>((pixel_for_basic_color(color) >> 8u) & 0xFF00u),
      static_cast<uint16_t>(pixel_for_basic_color(color) & 0xFF00u),
      static_cast<uint16_t>((pixel_for_basic_color(color) << 8u) & 0xFF00u),
  };
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::rgb_fore_color(const RGBColor &color) {
  NF_TOOLBOX_TRAP(kTrapRGBForeColor);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->rgb_fg_color = color;
  port->fg_pixel = pixel_for_rgb(color);
  port->fg_color = static_cast<int32_t>(port->fg_pixel);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::rgb_back_color(const RGBColor &color) {
  NF_TOOLBOX_TRAP(kTrapRGBBackColor);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->rgb_bk_color = color;
  port->bk_pixel = pixel_for_rgb(color);
  port->bk_color = static_cast<int32_t>(port->bk_pixel);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::frame_rect(const Rect &rect) {
  NF_TOOLBOX_TRAP(kTrapFrameRect);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }
  if (empty_rect(rect)) {
    return NF_OK;
  }

  const uint32_t color = port->fg_pixel;
  const int16_t top = clamp_low(rect.top);
  const int16_t left = clamp_low(rect.left);
  const int16_t bottom = clamp_high(rect.bottom, static_cast<int16_t>(kMaxModeledBitmapHeight));
  const int16_t right = clamp_high(rect.right, static_cast<int16_t>(kMaxModeledBitmapWidth));

  for (int16_t h = left; h < right; ++h) {
    port->pixels[(static_cast<size_t>(top) * kMaxModeledBitmapWidth) + static_cast<size_t>(h)] = color;
    port->pixels[(static_cast<size_t>(bottom - 1) * kMaxModeledBitmapWidth) + static_cast<size_t>(h)] = color;
  }
  for (int16_t v = top; v < bottom; ++v) {
    port->pixels[(static_cast<size_t>(v) * kMaxModeledBitmapWidth) + static_cast<size_t>(left)] = color;
    port->pixels[(static_cast<size_t>(v) * kMaxModeledBitmapWidth) + static_cast<size_t>(right - 1)] = color;
  }
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::paint_rect(const Rect &rect) {
  NF_TOOLBOX_TRAP(kTrapPaintRect);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  return fill_rect(rect, port->pn_pat);
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::erase_rect(const Rect &rect) {
  NF_TOOLBOX_TRAP(kTrapEraseRect);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  return fill_rect(rect, port->bk_pat);
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::invert_rect(const Rect &rect) {
  NF_TOOLBOX_TRAP(kTrapInvertRect);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }
  if (empty_rect(rect)) {
    return NF_OK;
  }

  const int16_t top = clamp_low(rect.top);
  const int16_t left = clamp_low(rect.left);
  const int16_t bottom = clamp_high(rect.bottom, static_cast<int16_t>(kMaxModeledBitmapHeight));
  const int16_t right = clamp_high(rect.right, static_cast<int16_t>(kMaxModeledBitmapWidth));

  for (int16_t v = top; v < bottom; ++v) {
    for (int16_t h = left; h < right; ++h) {
      const size_t index = (static_cast<size_t>(v) * kMaxModeledBitmapWidth) + static_cast<size_t>(h);
      port->pixels[index] ^= kWhitePixel;
    }
  }
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Rectangle Drawing".
nf_status PortState::fill_rect(const Rect &rect, const Pattern &pattern) {
  NF_TOOLBOX_TRAP(kTrapFillRect);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }
  if (empty_rect(rect)) {
    return NF_OK;
  }

  const uint32_t foreground = port->fg_pixel;
  const uint32_t background = port->bk_pixel;
  const int16_t top = clamp_low(rect.top);
  const int16_t left = clamp_low(rect.left);
  const int16_t bottom = clamp_high(rect.bottom, static_cast<int16_t>(kMaxModeledBitmapHeight));
  const int16_t right = clamp_high(rect.right, static_cast<int16_t>(kMaxModeledBitmapWidth));

  for (int16_t v = top; v < bottom; ++v) {
    for (int16_t h = left; h < right; ++h) {
      port->pixels[(static_cast<size_t>(v) * kMaxModeledBitmapWidth) + static_cast<size_t>(h)] =
          pattern_pixel(pattern, v, h, foreground, background);
    }
  }
  return NF_OK;
}

#undef NF_TOOLBOX_TRAP

}  // namespace nightfall::quickdraw
