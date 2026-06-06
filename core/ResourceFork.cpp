#include "ResourceFork.h"

#include <stddef.h>

namespace nightfall::resource {

namespace {

constexpr size_t kResourceForkHeaderSize = 16u;
constexpr size_t kResourceMapHeaderSize = 28u;
constexpr size_t kTypeEntrySize = 8u;
constexpr size_t kReferenceEntrySize = 12u;
constexpr uint32_t kResourceDataOffsetMask = 0x00FFFFFFu;

bool checked_range(size_t offset, size_t length, size_t byte_count) {
  return offset <= byte_count && length <= byte_count - offset;
}

uint16_t read_u16(const uint8_t *bytes) {
  return static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8u) | bytes[1]);
}

int16_t read_i16(const uint8_t *bytes) {
  return static_cast<int16_t>(read_u16(bytes));
}

uint32_t read_u24(const uint8_t *bytes) {
  return (static_cast<uint32_t>(bytes[0]) << 16u) | (static_cast<uint32_t>(bytes[1]) << 8u) |
         static_cast<uint32_t>(bytes[2]);
}

uint32_t read_u32(const uint8_t *bytes) {
  return (static_cast<uint32_t>(bytes[0]) << 24u) | (static_cast<uint32_t>(bytes[1]) << 16u) |
         (static_cast<uint32_t>(bytes[2]) << 8u) | static_cast<uint32_t>(bytes[3]);
}

bool copy_resource_name(const uint8_t *bytes,
                        size_t byte_count,
                        size_t name_list_offset,
                        int16_t name_offset,
                        std::string *out_name) {
  out_name->clear();
  if (name_offset < 0) {
    return true;
  }

  const size_t absolute = name_list_offset + static_cast<size_t>(name_offset);
  if (!checked_range(absolute, 1u, byte_count)) {
    return false;
  }

  const size_t length = bytes[absolute];
  if (!checked_range(absolute + 1u, length, byte_count)) {
    return false;
  }

  out_name->assign(reinterpret_cast<const char *>(bytes + absolute + 1u), length);
  return true;
}

}  // namespace

uint32_t make_type_code(char a, char b, char c, char d) {
  return (static_cast<uint32_t>(static_cast<uint8_t>(a)) << 24u) |
         (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 16u) |
         (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 8u) |
         static_cast<uint32_t>(static_cast<uint8_t>(d));
}

std::string type_code_to_string(uint32_t type) {
  std::string result(4u, ' ');
  result[0] = static_cast<char>((type >> 24u) & 0xFFu);
  result[1] = static_cast<char>((type >> 16u) & 0xFFu);
  result[2] = static_cast<char>((type >> 8u) & 0xFFu);
  result[3] = static_cast<char>(type & 0xFFu);
  return result;
}

// Source: docs/clean-room-sources.md, "Phase 5 Resource Fork Runner Backfill".
nf_status parse_resource_fork(const uint8_t *bytes, size_t byte_count, ResourceFork *out_fork) {
  if (bytes == nullptr || out_fork == nullptr || !checked_range(0u, kResourceForkHeaderSize, byte_count)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  const uint32_t data_offset = read_u32(bytes);
  const uint32_t map_offset = read_u32(bytes + 4u);
  const uint32_t data_length = read_u32(bytes + 8u);
  const uint32_t map_length = read_u32(bytes + 12u);

  if (!checked_range(data_offset, data_length, byte_count) || !checked_range(map_offset, map_length, byte_count) ||
      !checked_range(map_offset, kResourceMapHeaderSize, byte_count)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  ResourceFork parsed{};
  parsed.file_attributes = read_u16(bytes + map_offset + 22u);
  const uint16_t type_list_relative = read_u16(bytes + map_offset + 24u);
  const uint16_t name_list_relative = read_u16(bytes + map_offset + 26u);
  const size_t type_list_offset = map_offset + type_list_relative;
  const size_t name_list_offset = map_offset + name_list_relative;

  if (!checked_range(type_list_offset, 2u, byte_count) || !checked_range(name_list_offset, 0u, byte_count)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  const uint16_t type_count = static_cast<uint16_t>(read_u16(bytes + type_list_offset) + 1u);
  if (!checked_range(type_list_offset + 2u, static_cast<size_t>(type_count) * kTypeEntrySize, byte_count)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (uint16_t type_index = 0u; type_index < type_count; ++type_index) {
    const size_t type_entry_offset = type_list_offset + 2u + static_cast<size_t>(type_index) * kTypeEntrySize;
    const uint32_t type = read_u32(bytes + type_entry_offset);
    const uint16_t resource_count = static_cast<uint16_t>(read_u16(bytes + type_entry_offset + 4u) + 1u);
    const uint16_t reference_list_relative = read_u16(bytes + type_entry_offset + 6u);
    const size_t reference_list_offset = type_list_offset + reference_list_relative;

    if (!checked_range(reference_list_offset, static_cast<size_t>(resource_count) * kReferenceEntrySize, byte_count)) {
      return NF_ERROR_INVALID_ARGUMENT;
    }

    for (uint16_t resource_index = 0u; resource_index < resource_count; ++resource_index) {
      const size_t ref_offset = reference_list_offset + static_cast<size_t>(resource_index) * kReferenceEntrySize;
      const int16_t id = read_i16(bytes + ref_offset);
      const int16_t name_offset = read_i16(bytes + ref_offset + 2u);
      const uint8_t attributes = bytes[ref_offset + 4u];
      const uint32_t resource_data_relative = read_u24(bytes + ref_offset + 5u) & kResourceDataOffsetMask;
      const size_t resource_data_header_offset = data_offset + resource_data_relative;

      if (!checked_range(resource_data_header_offset, 4u, byte_count)) {
        return NF_ERROR_INVALID_ARGUMENT;
      }

      const uint32_t resource_length = read_u32(bytes + resource_data_header_offset);
      const size_t resource_data_offset = resource_data_header_offset + 4u;
      if (!checked_range(resource_data_offset, resource_length, byte_count)) {
        return NF_ERROR_INVALID_ARGUMENT;
      }

      ResourceEntry entry{};
      entry.type = type;
      entry.id = id;
      entry.attributes = attributes;
      if (!copy_resource_name(bytes, byte_count, name_list_offset, name_offset, &entry.name)) {
        return NF_ERROR_INVALID_ARGUMENT;
      }
      entry.data.assign(bytes + resource_data_offset, bytes + resource_data_offset + resource_length);
      parsed.entries.push_back(entry);
    }
  }

  *out_fork = parsed;
  return NF_OK;
}

const ResourceEntry *find_resource(const ResourceFork &fork, uint32_t type, int16_t id) {
  for (const ResourceEntry &entry : fork.entries) {
    if (entry.type == type && entry.id == id) {
      return &entry;
    }
  }
  return nullptr;
}

size_t count_type(const ResourceFork &fork, uint32_t type) {
  size_t count = 0u;
  for (const ResourceEntry &entry : fork.entries) {
    if (entry.type == type) {
      ++count;
    }
  }
  return count;
}

}  // namespace nightfall::resource
