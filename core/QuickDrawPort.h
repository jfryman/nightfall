#ifndef NIGHTFALL_CORE_QUICKDRAWPORT_H
#define NIGHTFALL_CORE_QUICKDRAWPORT_H

#include "nightfall.h"

#include <stddef.h>
#include <stdint.h>

namespace nightfall::quickdraw {

constexpr uint16_t kTrapInitPort = 0xA86Du;
constexpr uint16_t kTrapInitGraf = 0xA86Eu;
constexpr uint16_t kTrapOpenPort = 0xA86Fu;
constexpr uint16_t kTrapSetPort = 0xA873u;
constexpr uint16_t kTrapGetPort = 0xA874u;
constexpr uint16_t kTrapSetPortBits = 0xA875u;
constexpr uint16_t kTrapClosePort = 0xA87Du;

constexpr uint32_t kNilAddress = 0u;
constexpr uint32_t kDefaultScreenWidth = 640u;
constexpr uint32_t kDefaultScreenHeight = 480u;
constexpr uint16_t kDefaultScreenRowBytes = static_cast<uint16_t>(kDefaultScreenWidth * 4u);
constexpr int16_t kPatCopy = 8;
constexpr int16_t kSrcOr = 1;
constexpr int32_t kBlackColor = 0x00000021;
constexpr int32_t kWhiteColor = 0x0000001E;

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
  int16_t colr_bit;
  int16_t pat_stretch;
  uint32_t pic_save;
  uint32_t rgn_save;
  uint32_t poly_save;
  uint32_t graf_procs;
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
  nf_status dispatch(uint16_t trap_word, uint32_t argument_address);

  const QuickDrawGlobals &globals() const;
  const GrafPort *find_port(uint32_t port_address) const;
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
