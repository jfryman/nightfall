#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include "doctest.h"

#include "nightfall.h"

#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>
#include <vector>

namespace {

struct CapturedTrace {
  const char *category;
  const char *name;
  uint64_t value;
};

void capture_trace(const nf_trace_event *event, void *user_data) {
  auto *events = static_cast<std::vector<CapturedTrace> *>(user_data);
  events->push_back(CapturedTrace{
      event->category,
      event->name,
      event->value,
  });
}

nf_status record_trap(nf_context *, uint16_t trap_word, void *user_data) {
  auto *seen_trap = static_cast<uint16_t *>(user_data);
  *seen_trap = trap_word;
  return NF_OK;
}

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

uint32_t type_code(char a, char b, char c, char d) {
  return (static_cast<uint32_t>(static_cast<uint8_t>(a)) << 24u) |
         (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 16u) |
         (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 8u) |
         static_cast<uint32_t>(static_cast<uint8_t>(d));
}

std::vector<uint8_t> make_adgm_resource_fork_fixture() {
  std::vector<uint8_t> bytes(128u, 0u);
  constexpr size_t data_offset = 0x10u;
  constexpr size_t map_offset = 0x30u;
  constexpr size_t type_list_offset = map_offset + 0x1Cu;
  constexpr size_t name_list_offset = map_offset + 0x34u;
  constexpr size_t reference_list_offset = type_list_offset + 10u;

  put_u32(&bytes, 0u, data_offset);
  put_u32(&bytes, 4u, map_offset);
  put_u32(&bytes, 8u, 8u);
  put_u32(&bytes, 12u, 0x40u);

  put_u32(&bytes, data_offset, 4u);
  bytes[data_offset + 4u] = 0xA9u;
  bytes[data_offset + 5u] = 0x75u;
  bytes[data_offset + 6u] = 0x4Eu;
  bytes[data_offset + 7u] = 0x75u;

  put_u32(&bytes, map_offset, data_offset);
  put_u32(&bytes, map_offset + 4u, map_offset);
  put_u32(&bytes, map_offset + 8u, 8u);
  put_u32(&bytes, map_offset + 12u, 0x40u);
  put_u16(&bytes, map_offset + 22u, 0u);
  put_u16(&bytes, map_offset + 24u, 0x1Cu);
  put_u16(&bytes, map_offset + 26u, 0x34u);

  put_u16(&bytes, type_list_offset, 0u);
  put_u32(&bytes, type_list_offset + 2u, type_code('A', 'D', 'g', 'm'));
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

TEST_CASE("core context lifecycle, traps, and fixture execution are deterministic") {
  const nf_version version = nf_get_version();
  CHECK(version.abi == NF_ABI_VERSION);

  CHECK(nf_context_create(nullptr, nullptr) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(nf_context_tick(nullptr, 1u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(nf_context_ticks(nullptr) == 0u);

  std::vector<CapturedTrace> events;
  nf_context_options options{
      capture_trace,
      &events,
  };

  nf_context *context = nullptr;
  REQUIRE(nf_context_create(&options, &context) == NF_OK);
  REQUIRE(context != nullptr);
  CHECK(nf_context_ticks(context) == 0u);

  CHECK(nf_context_tick(context, 4u) == NF_OK);
  CHECK(nf_context_tick(context, 2u) == NF_OK);
  CHECK(nf_context_ticks(context) == 6u);

  CHECK(nf_trap_is_aline(0xA000u) == 1);
  CHECK(nf_trap_is_aline(0xAFFFu) == 1);
  CHECK(nf_trap_is_aline(0x9000u) == 0);
  CHECK(nf_context_dispatch_trap(context, 0x4E75u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(nf_context_dispatch_trap(context, 0xA000u) == NF_ERROR_UNIMPLEMENTED);

  uint16_t seen_trap = 0u;
  CHECK(nf_context_register_trap(context, 0xA9FFu, record_trap, &seen_trap) == NF_OK);
  CHECK(nf_context_dispatch_trap(context, 0xA9FFu) == NF_OK);
  CHECK(seen_trap == 0xA9FFu);

  seen_trap = 0u;
  nf_execution_result result{};
  const uint8_t fixture_bytes[] = {
      0xA0u,
      0x00u,
      0x4Eu,
      0x75u,
  };
  CHECK(nf_context_register_trap(context, 0xA000u, record_trap, &seen_trap) == NF_OK);
  CHECK(nf_context_execute_fixture(context, fixture_bytes, sizeof(fixture_bytes), &result) == NF_OK);
  CHECK(seen_trap == 0xA000u);
  CHECK(result.bytes_consumed == 4u);
  CHECK(result.last_word == 0x4E75u);
  CHECK(result.status == NF_OK);

  const uint8_t unsupported_fixture[] = {
      0x00u,
      0x00u,
  };
  CHECK(nf_context_execute_fixture(context, unsupported_fixture, sizeof(unsupported_fixture), &result) ==
        NF_ERROR_UNSUPPORTED_INSTRUCTION);
  CHECK(result.bytes_consumed == 2u);
  CHECK(result.last_word == 0x0000u);
  CHECK(result.status == NF_ERROR_UNSUPPORTED_INSTRUCTION);

  const uint8_t incomplete_fixture[] = {
      0x4Eu,
      0x71u,
  };
  CHECK(nf_context_execute_fixture(context, incomplete_fixture, sizeof(incomplete_fixture), &result) ==
        NF_ERROR_UNSUPPORTED_INSTRUCTION);

  CHECK(nf_context_execute_fixture(context, fixture_bytes, 0u, &result) == NF_ERROR_INCOMPLETE_PROGRAM);
  CHECK(result.bytes_consumed == 0u);
  CHECK(result.last_word == 0u);
  CHECK(result.status == NF_ERROR_INCOMPLETE_PROGRAM);

  nf_context_destroy(context);

  nf_context *pool_contexts[4]{};
  for (nf_context *&pool_context : pool_contexts) {
    CHECK(nf_context_create(nullptr, &pool_context) == NF_OK);
    REQUIRE(pool_context != nullptr);
  }
  nf_context *exhausted_context = nullptr;
  CHECK(nf_context_create(nullptr, &exhausted_context) == NF_ERROR_RESOURCE_EXHAUSTED);
  CHECK(exhausted_context == nullptr);
  for (nf_context *pool_context : pool_contexts) {
    nf_context_destroy(pool_context);
  }

  REQUIRE(events.size() == 13u);
  CHECK(std::strcmp(events[0].name, "context.create") == 0);
  CHECK(std::strcmp(events[1].name, "context.tick") == 0);
  CHECK(events[1].value == 4u);
  CHECK(std::strcmp(events[2].name, "context.tick") == 0);
  CHECK(events[2].value == 6u);
  CHECK(std::strcmp(events[3].name, "trap.unimplemented") == 0);
  CHECK(events[3].value == 0xA000u);
  CHECK(std::strcmp(events[4].name, "trap.register") == 0);
  CHECK(events[4].value == 0xA9FFu);
  CHECK(std::strcmp(events[5].name, "trap.dispatch") == 0);
  CHECK(events[5].value == 0xA9FFu);
  CHECK(std::strcmp(events[6].name, "trap.register") == 0);
  CHECK(events[6].value == 0xA000u);
  CHECK(std::strcmp(events[7].name, "trap.dispatch") == 0);
  CHECK(events[7].value == 0xA000u);
  CHECK(std::strcmp(events[8].name, "fixture.return") == 0);
  CHECK(events[8].value == 0x4E75u);
  CHECK(std::strcmp(events[9].name, "fixture.unsupported") == 0);
  CHECK(events[9].value == 0x0000u);
  CHECK(std::strcmp(events[10].name, "fixture.unsupported") == 0);
  CHECK(events[10].value == 0x4E71u);
  CHECK(std::strcmp(events[11].name, "fixture.incomplete") == 0);
  CHECK(events[11].value == 0u);
  CHECK(std::strcmp(events[12].name, "context.destroy") == 0);
}

TEST_CASE("module C ABI loads ADgm resource fork and exposes headless framebuffer") {
  std::vector<CapturedTrace> events;
  nf_context_options options{
      capture_trace,
      &events,
  };

  nf_context *context = nullptr;
  REQUIRE(nf_context_create(&options, &context) == NF_OK);
  REQUIRE(context != nullptr);
  CHECK(nf_context_set_random_seed(context, 0x00C0FFEEu) == NF_OK);

  char path[] = "/tmp/nightfall-adgm-fixture-XXXXXX";
  const int fd = mkstemp(path);
  REQUIRE(fd >= 0);
  close(fd);

  const std::vector<uint8_t> resource_fork = make_adgm_resource_fork_fixture();
  REQUIRE(setxattr(path, "com.apple.ResourceFork", resource_fork.data(), resource_fork.size(), 0u, 0) == 0);

  CHECK(nf_module_load(context, path) == NF_OK);
  CHECK(nf_module_start(context, 64u, 32u) == NF_OK);
  CHECK(nf_advance(context, 10u) == NF_OK);
  CHECK(nf_context_ticks(context) == 10u);

  const uint8_t *pixels = nullptr;
  uint32_t width = 0u;
  uint32_t height = 0u;
  uint32_t stride = 0u;
  CHECK(nf_module_framebuffer(context, &pixels, &width, &height, &stride) == NF_OK);
  REQUIRE(pixels != nullptr);
  CHECK(width == 64u);
  CHECK(height == 32u);
  CHECK(stride == 256u);
  CHECK(pixels[0] == 0u);
  CHECK(pixels[(static_cast<size_t>(height) - 1u) * stride + stride - 1u] == 0u);

  CHECK(nf_module_stop(context) == NF_OK);
  CHECK(nf_module_framebuffer(context, &pixels, &width, &height, &stride) == NF_ERROR_INVALID_ARGUMENT);

  nf_context_destroy(context);
  std::remove(path);

  REQUIRE(events.size() >= 7u);
  CHECK(std::strcmp(events[0].name, "context.create") == 0);
  CHECK(std::strcmp(events[1].name, "context.random_seed") == 0);
  CHECK(std::strcmp(events[2].name, "load.adgm0") == 0);
  CHECK(std::strcmp(events[3].name, "start.traps") == 0);
  CHECK(std::strcmp(events[4].name, "start.unimplemented_traps") == 0);
  CHECK(std::strcmp(events[5].name, "start") == 0);
  CHECK(std::strcmp(events[6].name, "advance") == 0);
}
