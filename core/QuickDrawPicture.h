#ifndef NIGHTFALL_CORE_QUICKDRAWPICTURE_H
#define NIGHTFALL_CORE_QUICKDRAWPICTURE_H

#include "QuickDrawPort.h"

#include <stddef.h>
#include <stdint.h>

namespace nightfall::quickdraw {

constexpr uint16_t kPictVersionOp = 0x0011u;
constexpr uint16_t kPictVersion2 = 0x02FFu;
constexpr uint16_t kPictHeaderOp = 0x0C00u;
constexpr uint16_t kPictNop = 0x0000u;
constexpr uint16_t kPictRGBForeColor = 0x001Au;
constexpr uint16_t kPictRGBBackColor = 0x001Bu;
constexpr uint16_t kPictLine = 0x0020u;
constexpr uint16_t kPictFrameRect = 0x0030u;
constexpr uint16_t kPictPaintRect = 0x0031u;
constexpr uint16_t kPictEraseRect = 0x0032u;
constexpr uint16_t kPictInvertRect = 0x0033u;
constexpr uint16_t kPictFillRect = 0x0034u;
constexpr uint16_t kPictEndPic = 0x00FFu;

struct PictOpcodeCoverage {
  Rect frame;
  size_t opcode_count;
  bool saw_version;
  bool saw_header;
  bool saw_nop;
  bool saw_rgb_fore_color;
  bool saw_rgb_back_color;
  bool saw_line;
  bool saw_frame_rect;
  bool saw_paint_rect;
  bool saw_erase_rect;
  bool saw_invert_rect;
  bool saw_fill_rect;
  bool saw_end_pic;
};

nf_status validate_pict2(const uint8_t *bytes, size_t byte_count, PictOpcodeCoverage *out_coverage);

}  // namespace nightfall::quickdraw

#endif
