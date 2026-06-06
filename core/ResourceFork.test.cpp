#include "ResourceFork.h"

#include "doctest.h"

#include <stdint.h>

#include <vector>

namespace {

void put_u16(std::vector<uint8_t> *bytes, size_t offset, uint16_t value) {
  (*bytes)[offset] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
  (*bytes)[offset + 1u] = static_cast<uint8_t>(value & 0xFFu);
}

void put_u24(std::vector<uint8_t> *bytes, size_t offset, uint32_t value) {
  (*bytes)[offset] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
  (*bytes)[offset + 1u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
  (*bytes)[offset + 2u] = static_cast<uint8_t>(value & 0xFFu);
}

void put_u32(std::vector<uint8_t> *bytes, size_t offset, uint32_t value) {
  (*bytes)[offset] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
  (*bytes)[offset + 1u] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
  (*bytes)[offset + 2u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
  (*bytes)[offset + 3u] = static_cast<uint8_t>(value & 0xFFu);
}

std::vector<uint8_t> make_resource_fork_fixture() {
  std::vector<uint8_t> bytes(128u, 0u);
  constexpr size_t data_offset = 0x10u;
  constexpr size_t map_offset = 0x30u;
  constexpr size_t type_list_offset = map_offset + 0x1Cu;
  constexpr size_t name_list_offset = map_offset + 0x34u;
  constexpr size_t reference_list_offset = type_list_offset + 10u;

  put_u32(&bytes, 0u, data_offset);
  put_u32(&bytes, 4u, map_offset);
  put_u32(&bytes, 8u, 0x0Cu);
  put_u32(&bytes, 12u, 0x40u);

  put_u32(&bytes, data_offset, 8u);
  bytes[data_offset + 4u] = 0x60u;
  bytes[data_offset + 5u] = 0x02u;
  bytes[data_offset + 6u] = 0xA9u;
  bytes[data_offset + 7u] = 0xA0u;
  bytes[data_offset + 8u] = 0x4Eu;
  bytes[data_offset + 9u] = 0x75u;
  bytes[data_offset + 10u] = 0x12u;
  bytes[data_offset + 11u] = 0x34u;

  put_u32(&bytes, map_offset, data_offset);
  put_u32(&bytes, map_offset + 4u, map_offset);
  put_u32(&bytes, map_offset + 8u, 0x0Cu);
  put_u32(&bytes, map_offset + 12u, 0x40u);
  put_u16(&bytes, map_offset + 22u, 0u);
  put_u16(&bytes, map_offset + 24u, 0x1Cu);
  put_u16(&bytes, map_offset + 26u, 0x34u);

  put_u16(&bytes, type_list_offset, 0u);
  put_u32(&bytes, type_list_offset + 2u, nightfall::resource::make_type_code('A', 'D', 'g', 'm'));
  put_u16(&bytes, type_list_offset + 6u, 0u);
  put_u16(&bytes, type_list_offset + 8u, static_cast<uint16_t>(reference_list_offset - type_list_offset));

  put_u16(&bytes, reference_list_offset, 0);
  put_u16(&bytes, reference_list_offset + 2u, 0);
  bytes[reference_list_offset + 4u] = 0x10u;
  put_u24(&bytes, reference_list_offset + 5u, 0u);

  bytes[name_list_offset] = 4u;
  bytes[name_list_offset + 1u] = 'm';
  bytes[name_list_offset + 2u] = 'a';
  bytes[name_list_offset + 3u] = 'i';
  bytes[name_list_offset + 4u] = 'n';

  return bytes;
}

}  // namespace

TEST_CASE("resource fork parser indexes type/id data and names") {
  const std::vector<uint8_t> bytes = make_resource_fork_fixture();
  nightfall::resource::ResourceFork fork{};
  CHECK(nightfall::resource::parse_resource_fork(bytes.data(), bytes.size(), &fork) == NF_OK);
  REQUIRE(fork.entries.size() == 1u);

  const uint32_t adgm = nightfall::resource::make_type_code('A', 'D', 'g', 'm');
  CHECK(nightfall::resource::count_type(fork, adgm) == 1u);
  const nightfall::resource::ResourceEntry *entry = nightfall::resource::find_resource(fork, adgm, 0);
  REQUIRE(entry != nullptr);
  CHECK(entry->attributes == 0x10u);
  CHECK(entry->name == "main");
  REQUIRE(entry->data.size() == 8u);
  CHECK(entry->data[0] == 0x60u);
  CHECK(entry->data[2] == 0xA9u);
}

TEST_CASE("resource fork parser rejects truncated data") {
  std::vector<uint8_t> bytes = make_resource_fork_fixture();
  bytes.resize(20u);
  nightfall::resource::ResourceFork fork{};
  CHECK(nightfall::resource::parse_resource_fork(bytes.data(), bytes.size(), &fork) == NF_ERROR_INVALID_ARGUMENT);
}
