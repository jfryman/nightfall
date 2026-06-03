#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include "doctest.h"

#include "nightfall.h"

#include <cstring>
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
