#include "QuickDrawPort.h"

namespace nightfall::quickdraw {

#define NF_TOOLBOX_TRAP(trap_word) record_trace(trap_word)

namespace {

constexpr Pattern kWhitePattern{{0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u}};
constexpr Pattern kBlackPattern{{0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}};
constexpr Pattern kGrayPattern{{0xAAu, 0x55u, 0xAAu, 0x55u, 0xAAu, 0x55u, 0xAAu, 0x55u}};
constexpr Pattern kLightGrayPattern{{0x88u, 0x22u, 0x88u, 0x22u, 0x88u, 0x22u, 0x88u, 0x22u}};
constexpr Pattern kDarkGrayPattern{{0xDDu, 0x77u, 0xDDu, 0x77u, 0xDDu, 0x77u, 0xDDu, 0x77u}};

constexpr Rect kWideOpenClip{-32768, -32768, 32767, 32767};

BitMap default_screen_bits() {
  return BitMap{
      0u,
      kDefaultScreenRowBytes,
      Rect{0, 0, static_cast<int16_t>(kDefaultScreenHeight), static_cast<int16_t>(kDefaultScreenWidth)},
  };
}

bool is_nil(uint32_t address) {
  return address == kNilAddress;
}

}  // namespace

// Source: docs/clean-room-sources.md, "QuickDraw Port State".
nf_status PortState::init_graf(uint32_t global_ptr) {
  NF_TOOLBOX_TRAP(kTrapInitGraf);

  globals_ = QuickDrawGlobals{
      true,
      global_ptr,
      kNilAddress,
      kWhitePattern,
      kBlackPattern,
      kGrayPattern,
      kLightGrayPattern,
      kDarkGrayPattern,
      default_screen_bits(),
      1,
  };
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Port State".
nf_status PortState::open_port(uint32_t port_address) {
  NF_TOOLBOX_TRAP(kTrapOpenPort);

  if (is_nil(port_address)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  GrafPort *port = find_or_create_port(port_address);
  if (port == nullptr) {
    return NF_ERROR_RESOURCE_EXHAUSTED;
  }

  const BitMap screen_bits = globals_.screen_bits;
  *port = GrafPort{
      port_address,
      true,
      0,
      screen_bits,
      screen_bits.bounds,
      Region{true, screen_bits.bounds},
      Region{true, kWideOpenClip},
      kWhitePattern,
      kBlackPattern,
      Point{0, 0},
      Point{1, 1},
      kPatCopy,
      kBlackPattern,
      0,
      0,
      0,
      kSrcOr,
      0,
      0,
      kBlackColor,
      kWhiteColor,
      RGBColor{0x0000u, 0x0000u, 0x0000u},
      RGBColor{0xFFFFu, 0xFFFFu, 0xFFFFu},
      kBlackPixel,
      kWhitePixel,
      0,
      0,
      kNilAddress,
      kNilAddress,
      kNilAddress,
      kNilAddress,
      {},
  };
  return set_port(port_address);
}

// Source: docs/clean-room-sources.md, "QuickDraw Port State".
nf_status PortState::close_port(uint32_t port_address) {
  NF_TOOLBOX_TRAP(kTrapClosePort);

  GrafPort *port = find_port_mutable(port_address);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->vis_rgn.allocated = false;
  port->clip_rgn.allocated = false;
  port->open = false;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Port State".
nf_status PortState::set_port(uint32_t port_address) {
  NF_TOOLBOX_TRAP(kTrapSetPort);

  if (is_nil(port_address) || find_port(port_address) == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  globals_.the_port = port_address;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Port State".
nf_status PortState::get_port(uint32_t *out_port_address) const {
  NF_TOOLBOX_TRAP(kTrapGetPort);

  if (out_port_address == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_port_address = globals_.the_port;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "QuickDraw Port State".
nf_status PortState::set_port_bits(const BitMap &bitmap) {
  NF_TOOLBOX_TRAP(kTrapSetPortBits);

  GrafPort *port = find_port_mutable(globals_.the_port);
  if (port == nullptr || !port->open) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  port->port_bits = bitmap;
  return NF_OK;
}

nf_status PortState::dispatch(uint16_t trap_word, uint32_t argument_address) {
  switch (trap_word) {
    case kTrapInitGraf:
      return init_graf(argument_address);
    case kTrapOpenPort:
      return open_port(argument_address);
    case kTrapClosePort:
      return close_port(argument_address);
    case kTrapSetPort:
      return set_port(argument_address);
    case kTrapSetPortBits:
      return set_port_bits(BitMap{
          argument_address,
          kDefaultScreenRowBytes,
          Rect{0, 0, static_cast<int16_t>(kDefaultScreenHeight), static_cast<int16_t>(kDefaultScreenWidth)},
      });
    case kTrapGetPort:
      return argument_address == kNilAddress ? NF_ERROR_INVALID_ARGUMENT : NF_OK;
    default:
      return NF_ERROR_UNIMPLEMENTED;
  }
}

const QuickDrawGlobals &PortState::globals() const {
  return globals_;
}

const GrafPort *PortState::find_port(uint32_t port_address) const {
  for (const GrafPort &port : ports_) {
    if (port.address == port_address) {
      return &port;
    }
  }
  return nullptr;
}

uint32_t PortState::pixel(uint32_t port_address, int16_t v, int16_t h) const {
  const GrafPort *port = find_port(port_address);
  if (port == nullptr || v < 0 || h < 0) {
    return 0u;
  }

  const auto row = static_cast<size_t>(v);
  const auto column = static_cast<size_t>(h);
  if (row >= kMaxModeledBitmapHeight || column >= kMaxModeledBitmapWidth) {
    return 0u;
  }

  return port->pixels[(row * kMaxModeledBitmapWidth) + column];
}

size_t PortState::trace_count() const {
  return trace_count_;
}

uint16_t PortState::trace_word(size_t index) const {
  if (index >= trace_count_) {
    return 0u;
  }
  return trace_words_[index];
}

GrafPort *PortState::find_or_create_port(uint32_t port_address) {
  if (GrafPort *existing = find_port_mutable(port_address); existing != nullptr) {
    return existing;
  }

  for (GrafPort &port : ports_) {
    if (port.address == kNilAddress) {
      port.address = port_address;
      return &port;
    }
  }
  return nullptr;
}

GrafPort *PortState::find_port_mutable(uint32_t port_address) {
  for (GrafPort &port : ports_) {
    if (port.address == port_address) {
      return &port;
    }
  }
  return nullptr;
}

void PortState::record_trace(uint16_t trap_word) const {
  if (trace_count_ >= kMaxTraceWords) {
    return;
  }
  trace_words_[trace_count_] = trap_word;
  ++trace_count_;
}

#undef NF_TOOLBOX_TRAP

}  // namespace nightfall::quickdraw
