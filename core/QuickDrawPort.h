#ifndef NIGHTFALL_CORE_QUICKDRAWPORT_H
#define NIGHTFALL_CORE_QUICKDRAWPORT_H

#include "nightfall.h"

#include <stddef.h>
#include <stdint.h>

namespace nightfall::quickdraw {

constexpr uint16_t kTrapInitPort = 0xA86Du;
constexpr uint16_t kTrapInitGraf = 0xA86Eu;
constexpr uint16_t kTrapOpenPort = 0xA86Fu;
constexpr uint16_t kTrapForeColor = 0xA862u;
constexpr uint16_t kTrapBackColor = 0xA863u;
constexpr uint16_t kTrapSetPort = 0xA873u;
constexpr uint16_t kTrapGetPort = 0xA874u;
constexpr uint16_t kTrapSetPortBits = 0xA875u;
constexpr uint16_t kTrapClosePort = 0xA87Du;
constexpr uint16_t kTrapFrameRect = 0xA8A1u;
constexpr uint16_t kTrapPaintRect = 0xA8A2u;
constexpr uint16_t kTrapEraseRect = 0xA8A3u;
constexpr uint16_t kTrapInvertRect = 0xA8A4u;
constexpr uint16_t kTrapFillRect = 0xA8A5u;
constexpr uint16_t kTrapLineTo = 0xA891u;
constexpr uint16_t kTrapLine = 0xA892u;
constexpr uint16_t kTrapMoveTo = 0xA893u;
constexpr uint16_t kTrapMove = 0xA894u;
constexpr uint16_t kTrapRGBForeColor = 0xAA14u;
constexpr uint16_t kTrapRGBBackColor = 0xAA15u;

constexpr uint32_t kNilAddress = 0u;
constexpr uint32_t kDefaultScreenWidth = 640u;
constexpr uint32_t kDefaultScreenHeight = 480u;
constexpr uint16_t kDefaultScreenRowBytes = static_cast<uint16_t>(kDefaultScreenWidth * 4u);
constexpr uint16_t kMaxModeledBitmapWidth = 64u;
constexpr uint16_t kMaxModeledBitmapHeight = 64u;
constexpr size_t kMaxModeledPixels = static_cast<size_t>(kMaxModeledBitmapWidth) * kMaxModeledBitmapHeight;
constexpr int16_t kPatCopy = 8;
constexpr int16_t kSrcOr = 1;
constexpr int32_t kBlackColor = 0x00000021;
constexpr int32_t kWhiteColor = 0x0000001E;
constexpr int32_t kRedColor = 0x000000CD;
constexpr int32_t kGreenColor = 0x00000155;
constexpr int32_t kBlueColor = 0x00000199;
constexpr uint32_t kBlackPixel = 0x00000000u;
constexpr uint32_t kWhitePixel = 0x00FFFFFFu;

struct Point {
  int16_t v;
  int16_t h;
};

struct Rect {
  int16_t top;
  int16_t left;
  int16_t bottom;
  int16_t right;
};

struct Pattern {
  uint8_t bytes[8];
};

struct RGBColor {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
};

struct BitMap {
  uint32_t base_addr;
  uint16_t row_bytes;
  Rect bounds;
};

struct Region {
  bool allocated;
  Rect bounds;
};

struct GrafPort {
  uint32_t address;
  bool open;
  int16_t device;
  BitMap port_bits;
  Rect port_rect;
  Region vis_rgn;
  Region clip_rgn;
  Pattern bk_pat;
  Pattern fill_pat;
  Point pn_loc;
  Point pn_size;
  int16_t pn_mode;
  Pattern pn_pat;
  int16_t pn_vis;
  int16_t tx_font;
  int16_t tx_face;
  int16_t tx_mode;
  int16_t tx_size;
  int32_t sp_extra;
  int32_t fg_color;
  int32_t bk_color;
  RGBColor rgb_fg_color;
  RGBColor rgb_bk_color;
  uint32_t fg_pixel;
  uint32_t bk_pixel;
  int16_t colr_bit;
  int16_t pat_stretch;
  uint32_t pic_save;
  uint32_t rgn_save;
  uint32_t poly_save;
  uint32_t graf_procs;
  uint32_t pixels[kMaxModeledPixels];
};

struct QuickDrawGlobals {
  bool initialized;
  uint32_t global_ptr;
  uint32_t the_port;
  Pattern white;
  Pattern black;
  Pattern gray;
  Pattern lt_gray;
  Pattern dk_gray;
  BitMap screen_bits;
  int32_t rand_seed;
};

class PortState {
 public:
  nf_status init_graf(uint32_t global_ptr);
  nf_status open_port(uint32_t port_address);
  nf_status close_port(uint32_t port_address);
  nf_status set_port(uint32_t port_address);
  nf_status get_port(uint32_t *out_port_address) const;
  nf_status set_port_bits(const BitMap &bitmap);
  nf_status fore_color(int32_t color);
  nf_status back_color(int32_t color);
  nf_status rgb_fore_color(const RGBColor &color);
  nf_status rgb_back_color(const RGBColor &color);
  nf_status frame_rect(const Rect &rect);
  nf_status paint_rect(const Rect &rect);
  nf_status erase_rect(const Rect &rect);
  nf_status invert_rect(const Rect &rect);
  nf_status fill_rect(const Rect &rect, const Pattern &pattern);
  nf_status move_to(int16_t h, int16_t v);
  nf_status move(int16_t dh, int16_t dv);
  nf_status line_to(int16_t h, int16_t v);
  nf_status line(int16_t dh, int16_t dv);
  nf_status dispatch(uint16_t trap_word, uint32_t argument_address);

  const QuickDrawGlobals &globals() const;
  const GrafPort *find_port(uint32_t port_address) const;
  uint32_t pixel(uint32_t port_address, int16_t v, int16_t h) const;
  size_t trace_count() const;
  uint16_t trace_word(size_t index) const;

 private:
  static constexpr size_t kMaxPorts = 8u;
  static constexpr size_t kMaxTraceWords = 32u;

  GrafPort *find_or_create_port(uint32_t port_address);
  GrafPort *find_port_mutable(uint32_t port_address);
  void record_trace(uint16_t trap_word) const;

  QuickDrawGlobals globals_{};
  GrafPort ports_[kMaxPorts]{};
  mutable uint16_t trace_words_[kMaxTraceWords]{};
  mutable size_t trace_count_ = 0u;
};

}  // namespace nightfall::quickdraw

#endif
