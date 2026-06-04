#include "QuickDrawPicture.h"

#include <stddef.h>
#include <stdint.h>
#include <vector>

namespace {

void push_u16(std::vector<uint8_t> *bytes, uint16_t value) {
  bytes->push_back(static_cast<uint8_t>((value >> 8u) & 0xFFu));
  bytes->push_back(static_cast<uint8_t>(value & 0xFFu));
}

void push_rect(std::vector<uint8_t> *bytes, uint8_t seed, uint8_t limit) {
  const int16_t top = static_cast<int16_t>(seed % limit);
  const int16_t left = static_cast<int16_t>((seed / 2u) % limit);
  const int16_t bottom = static_cast<int16_t>(top + 1 + (seed % (limit - top)));
  const int16_t right = static_cast<int16_t>(left + 1 + (seed % (limit - left)));
  push_u16(bytes, static_cast<uint16_t>(top));
  push_u16(bytes, static_cast<uint16_t>(left));
  push_u16(bytes, static_cast<uint16_t>(bottom));
  push_u16(bytes, static_cast<uint16_t>(right));
}

void push_header(std::vector<uint8_t> *bytes) {
  push_u16(bytes, 0u);
  push_u16(bytes, 0u);
  push_u16(bytes, 0u);
  push_u16(bytes, 64u);
  push_u16(bytes, 64u);
  push_u16(bytes, nightfall::quickdraw::kPictVersionOp);
  push_u16(bytes, nightfall::quickdraw::kPictVersion2);
  push_u16(bytes, nightfall::quickdraw::kPictHeaderOp);
  for (int index = 0; index < 24; ++index) {
    bytes->push_back(index < 4 ? 0xFFu : 0u);
  }
}

std::vector<uint8_t> structured_picture(const uint8_t *data, size_t size) {
  std::vector<uint8_t> bytes;
  push_header(&bytes);

  for (size_t index = 0; index < size && index < 96u; ++index) {
    switch (data[index] % 8u) {
      case 0:
        push_u16(&bytes, nightfall::quickdraw::kPictNop);
        break;
      case 1:
        push_u16(&bytes, nightfall::quickdraw::kPictRGBForeColor);
        push_u16(&bytes, static_cast<uint16_t>(data[index] * 257u));
        push_u16(&bytes, static_cast<uint16_t>((data[index] ^ 0x55u) * 257u));
        push_u16(&bytes, static_cast<uint16_t>((data[index] ^ 0xAAu) * 257u));
        break;
      case 2:
        push_u16(&bytes, nightfall::quickdraw::kPictRGBBackColor);
        push_u16(&bytes, static_cast<uint16_t>((data[index] ^ 0x11u) * 257u));
        push_u16(&bytes, static_cast<uint16_t>((data[index] ^ 0x22u) * 257u));
        push_u16(&bytes, static_cast<uint16_t>((data[index] ^ 0x44u) * 257u));
        break;
      case 3:
        push_u16(&bytes, nightfall::quickdraw::kPictLine);
        push_rect(&bytes, data[index], 64u);
        break;
      case 4:
        push_u16(&bytes, nightfall::quickdraw::kPictFrameRect);
        push_rect(&bytes, data[index], 64u);
        break;
      case 5:
        push_u16(&bytes, nightfall::quickdraw::kPictPaintRect);
        push_rect(&bytes, data[index], 64u);
        break;
      case 6:
        push_u16(&bytes, nightfall::quickdraw::kPictEraseRect);
        push_rect(&bytes, data[index], 64u);
        break;
      default:
        push_u16(&bytes, (data[index] & 1u) == 0u ? nightfall::quickdraw::kPictInvertRect
                                                   : nightfall::quickdraw::kPictFillRect);
        push_rect(&bytes, data[index], 64u);
        break;
    }
  }

  push_u16(&bytes, nightfall::quickdraw::kPictEndPic);
  return bytes;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  nightfall::quickdraw::PictOpcodeCoverage coverage{};
  (void)nightfall::quickdraw::validate_pict2(data, size, &coverage);

  std::vector<uint8_t> structured = structured_picture(data, size);
  (void)nightfall::quickdraw::validate_pict2(structured.data(), structured.size(), &coverage);
  return 0;
}
