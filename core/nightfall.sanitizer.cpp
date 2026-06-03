#include "nightfall.h"

#include <cstdlib>

namespace {

void require(bool condition) {
  if (!condition) {
    std::abort();
  }
}

nf_status record_trap(nf_context *, uint16_t trap_word, void *user_data) {
  auto *seen_trap = static_cast<uint16_t *>(user_data);
  *seen_trap = trap_word;
  return NF_OK;
}

}  // namespace

int main() {
  nf_context *context = nullptr;
  require(nf_context_create(nullptr, &context) == NF_OK);
  require(context != nullptr);

  uint16_t seen_trap = 0u;
  require(nf_context_register_trap(context, 0xA000u, record_trap, &seen_trap) == NF_OK);

  const uint8_t fixture_bytes[] = {
      0xA0u,
      0x00u,
      0x4Eu,
      0x75u,
  };
  nf_execution_result result{};
  require(nf_context_execute_fixture(context, fixture_bytes, sizeof(fixture_bytes), &result) == NF_OK);
  require(seen_trap == 0xA000u);
  require(result.status == NF_OK);
  require(result.last_word == 0x4E75u);

  nf_context_destroy(context);
  return 0;
}
