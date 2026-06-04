#include "QuickDrawPort.h"

#include <cstdlib>

namespace nightfall::quickdraw {

#define NF_TOOLBOX_TRAP(trap_word) record_trace(trap_word)

namespace {

void put_pixel(GrafPort &port, int16_t h, int16_t v) {
  if (h < 0 || v < 0 || h >= static_cast<int16_t>(kMaxModeledBitmapWidth) ||
      v >= static_cast<int16_t>(kMaxModeledBitmapHeight)) {
    return;
  }

  port.pixels[(static_cast<size_t>(v) * kMaxModeledBitmapWidth) + static_cast<size_t>(h)] = port.fg_pixel;
}

void draw_line(GrafPort &port, Point from, Point to) {
  int x0 = from.h;
  int y0 = from.v;
  const int x1 = to.h;
  const int y1 = to.v;
  const int dx = std::abs(x1 - x0);
  const int sx = x0 < x1 ? 1 : -1;
  const int dy = -std::abs(y1 - y0);
  const int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;

  for (;;) {
    put_pixel(port, static_cast<int16_t>(x0), static_cast<int16_t>(y0));
    if (x0 == x1 && y0 == y1) {
      break;
    }

    const int twice_error = 2 * error;
    if (twice_error >= dy) {
      error += dy;
      x0 += sx;
    }
    if (twice_error <= dx) {
      error += dx;
      y0 += sy;
    }
  }
}

}  // namespace

// Source: docs/clean-room-sources.md, "QuickDraw Line Drawing".
nf_status PortState::move_to(int16_t h, int16_t v) {
  NF_TOOLBOX_TRAP(kTrapMoveTo);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->pn_loc = Point{v, h};
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Line Drawing".
nf_status PortState::move(int16_t dh, int16_t dv) {
  NF_TOOLBOX_TRAP(kTrapMove);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->pn_loc.h = static_cast<int16_t>(port->pn_loc.h + dh);
  port->pn_loc.v = static_cast<int16_t>(port->pn_loc.v + dv);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Line Drawing".
nf_status PortState::line_to(int16_t h, int16_t v) {
  NF_TOOLBOX_TRAP(kTrapLineTo);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  const Point destination{v, h};
  if (port->pn_vis == 0) {
    draw_line(*port, port->pn_loc, destination);
  }
  port->pn_loc = destination;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Line Drawing".
nf_status PortState::line(int16_t dh, int16_t dv) {
  NF_TOOLBOX_TRAP(kTrapLine);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  const int16_t h = static_cast<int16_t>(port->pn_loc.h + dh);
  const int16_t v = static_cast<int16_t>(port->pn_loc.v + dv);
  const Point destination{v, h};
  if (port->pn_vis == 0) {
    draw_line(*port, port->pn_loc, destination);
  }
  port->pn_loc = destination;
  return NF_OK;
}

#undef NF_TOOLBOX_TRAP

}  // namespace nightfall::quickdraw
