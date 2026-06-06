#include "QuickDrawPicture.h"

namespace nightfall::quickdraw {

#define NF_TOOLBOX_TRAP(trap_word) record_trace(trap_word)

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

bool read_rgb(Reader *reader, RGBColor *out_color) {
  return reader->read_u16(&out_color->red) && reader->read_u16(&out_color->green) &&
         reader->read_u16(&out_color->blue);
}

int16_t map_coordinate(int16_t value, int16_t source_min, int16_t source_max, int16_t destination_min,
                       int16_t destination_max) {
  const int source_span = source_max - source_min;
  const int destination_span = destination_max - destination_min;
  if (source_span == 0) {
    return destination_min;
  }
  return static_cast<int16_t>(destination_min +
                              (((static_cast<int>(value) - source_min) * destination_span) / source_span));
}

Point map_point(Point point, const Rect &frame, const Rect &destination) {
  return Point{
      map_coordinate(point.v, frame.top, frame.bottom, destination.top, destination.bottom),
      map_coordinate(point.h, frame.left, frame.right, destination.left, destination.right),
  };
}

Rect map_rect(Rect rect, const Rect &frame, const Rect &destination) {
  const Point top_left = map_point(Point{rect.top, rect.left}, frame, destination);
  const Point bottom_right = map_point(Point{rect.bottom, rect.right}, frame, destination);
  return Rect{top_left.v, top_left.h, bottom_right.v, bottom_right.h};
}

nf_status play_or_validate_pict2(PortState *state,
                                 const uint8_t *bytes,
                                 size_t byte_count,
                                 const Rect *destination_rect,
                                 PictOpcodeCoverage *out_coverage) {
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

  const Rect destination = destination_rect == nullptr ? frame : *destination_rect;
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
      case kPictRGBForeColor: {
        coverage.saw_rgb_fore_color = true;
        RGBColor color{};
        if (!read_rgb(&reader, &color)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const nf_status status = state->rgb_fore_color(color);
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
      case kPictRGBBackColor: {
        coverage.saw_rgb_back_color = true;
        RGBColor color{};
        if (!read_rgb(&reader, &color)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const nf_status status = state->rgb_back_color(color);
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
      case kPictLine: {
        coverage.saw_line = true;
        Rect line{};
        if (!reader.read_rect(&line)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const Point from = map_point(Point{line.top, line.left}, frame, destination);
          const Point to = map_point(Point{line.bottom, line.right}, frame, destination);
          nf_status status = state->move_to(from.h, from.v);
          if (status != NF_OK) {
            return status;
          }
          status = state->line_to(to.h, to.v);
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
      case kPictFrameRect: {
        coverage.saw_frame_rect = true;
        Rect rect{};
        if (!reader.read_rect(&rect)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const nf_status status = state->frame_rect(map_rect(rect, frame, destination));
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
      case kPictPaintRect: {
        coverage.saw_paint_rect = true;
        Rect rect{};
        if (!reader.read_rect(&rect)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const nf_status status = state->paint_rect(map_rect(rect, frame, destination));
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
      case kPictEraseRect: {
        coverage.saw_erase_rect = true;
        Rect rect{};
        if (!reader.read_rect(&rect)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const nf_status status = state->erase_rect(map_rect(rect, frame, destination));
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
      case kPictInvertRect: {
        coverage.saw_invert_rect = true;
        Rect rect{};
        if (!reader.read_rect(&rect)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const nf_status status = state->invert_rect(map_rect(rect, frame, destination));
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
      case kPictFillRect: {
        coverage.saw_fill_rect = true;
        Rect rect{};
        if (!reader.read_rect(&rect)) {
          return NF_ERROR_INVALID_ARGUMENT;
        }
        if (state != nullptr) {
          const nf_status status = state->paint_rect(map_rect(rect, frame, destination));
          if (status != NF_OK) {
            return status;
          }
        }
        break;
      }
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

}  // namespace

// Source: docs/clean-room-sources.md, "QuickDraw PICT Streams".
nf_status validate_pict2(const uint8_t *bytes, size_t byte_count, PictOpcodeCoverage *out_coverage) {
  return play_or_validate_pict2(nullptr, bytes, byte_count, nullptr, out_coverage);
}

// Source: docs/clean-room-sources.md, "QuickDraw PICT Streams".
nf_status PortState::draw_picture(const uint8_t *bytes,
                                  size_t byte_count,
                                  const Rect &destination_rect,
                                  PictOpcodeCoverage *out_coverage) {
  NF_TOOLBOX_TRAP(kTrapDrawPicture);
  return play_or_validate_pict2(this, bytes, byte_count, &destination_rect, out_coverage);
}

#undef NF_TOOLBOX_TRAP

}  // namespace nightfall::quickdraw
