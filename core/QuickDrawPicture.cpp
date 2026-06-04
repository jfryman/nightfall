#include "QuickDrawPicture.h"

namespace nightfall::quickdraw {

namespace {

class Reader {
 public:
  Reader(const uint8_t *bytes, size_t byte_count) : bytes_(bytes), byte_count_(byte_count) {}

  bool read_u16(uint16_t *out_value) {
    if (remaining() < 2u) {
      return false;
    }
    *out_value = static_cast<uint16_t>((static_cast<uint16_t>(bytes_[offset_]) << 8u) |
                                       static_cast<uint16_t>(bytes_[offset_ + 1u]));
    offset_ += 2u;
    return true;
  }

  bool read_i16(int16_t *out_value) {
    uint16_t raw = 0u;
    if (!read_u16(&raw)) {
      return false;
    }
    *out_value = static_cast<int16_t>(raw);
    return true;
  }

  bool skip(size_t count) {
    if (remaining() < count) {
      return false;
    }
    offset_ += count;
    return true;
  }

  bool read_rect(Rect *out_rect) {
    return read_i16(&out_rect->top) && read_i16(&out_rect->left) &&
           read_i16(&out_rect->bottom) && read_i16(&out_rect->right);
  }

  bool at_end() const { return offset_ == byte_count_; }
  size_t remaining() const { return byte_count_ - offset_; }

 private:
  const uint8_t *bytes_;
  size_t byte_count_;
  size_t offset_ = 0u;
};

void reset_coverage(PictOpcodeCoverage *coverage) {
  if (coverage != nullptr) {
    *coverage = PictOpcodeCoverage{};
  }
}

bool skip_rgb(Reader *reader) {
  return reader->skip(6u);
}

bool skip_line(Reader *reader) {
  return reader->skip(8u);
}

bool skip_rect(Reader *reader) {
  return reader->skip(8u);
}

}  // namespace

// Source: docs/clean-room-sources.md, "QuickDraw PICT Streams".
nf_status validate_pict2(const uint8_t *bytes, size_t byte_count, PictOpcodeCoverage *out_coverage) {
  reset_coverage(out_coverage);
  if (bytes == nullptr || byte_count < 16u) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  Reader reader(bytes, byte_count);
  uint16_t ignored_size = 0u;
  Rect frame{};
  uint16_t opcode = 0u;
  uint16_t version = 0u;
  if (!reader.read_u16(&ignored_size) || !reader.read_rect(&frame) ||
      !reader.read_u16(&opcode) || opcode != kPictVersionOp ||
      !reader.read_u16(&version) || version != kPictVersion2) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  PictOpcodeCoverage coverage{};
  coverage.frame = frame;
  coverage.saw_version = true;
  coverage.opcode_count = 1u;

  if (!reader.read_u16(&opcode) || opcode != kPictHeaderOp || !reader.skip(24u)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }
  coverage.saw_header = true;
  ++coverage.opcode_count;

  while (!reader.at_end()) {
    if (!reader.read_u16(&opcode)) {
      return NF_ERROR_INVALID_ARGUMENT;
    }
    ++coverage.opcode_count;

    switch (opcode) {
      case kPictNop:
        coverage.saw_nop = true;
        break;
      case kPictRGBForeColor:
        coverage.saw_rgb_fore_color = true;
        if (!skip_rgb(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictRGBBackColor:
        coverage.saw_rgb_back_color = true;
        if (!skip_rgb(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictLine:
        coverage.saw_line = true;
        if (!skip_line(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictFrameRect:
        coverage.saw_frame_rect = true;
        if (!skip_rect(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictPaintRect:
        coverage.saw_paint_rect = true;
        if (!skip_rect(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictEraseRect:
        coverage.saw_erase_rect = true;
        if (!skip_rect(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictInvertRect:
        coverage.saw_invert_rect = true;
        if (!skip_rect(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictFillRect:
        coverage.saw_fill_rect = true;
        if (!skip_rect(&reader)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        break;
      case kPictEndPic:
        coverage.saw_end_pic = true;
        if (!reader.at_end()) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (out_coverage != nullptr) {
          *out_coverage = coverage;
        }
        return NF_OK;
      default:
        return NF_ERROR_UNIMPLEMENTED;
    }
  }

  return NF_ERROR_INCOMPLETE_PROGRAM;
}

}  // namespace nightfall::quickdraw
