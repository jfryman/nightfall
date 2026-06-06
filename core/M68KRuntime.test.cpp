#include "M68KRuntime.h"

#include "ToolboxManagers.h"

#include "doctest.h"

#include <stdint.h>
#include <cstring>

TEST_CASE("runtime executes through implemented A-line trap and stops at RTS") {
  const uint8_t program[] = {
      0xA9u,
      0x75u,
      0x4Eu,
      0x75u,
  };
  nightfall::m68k::Runtime runtime;
  CHECK(runtime.load_program(program, sizeof(program)) == NF_OK);

  const nightfall::m68k::RuntimeResult result = runtime.run();
  CHECK(result.status == NF_OK);
  CHECK(std::strcmp(result.stop_reason, "rts") == 0);
  REQUIRE(result.traps.size() == 1u);
  CHECK(result.traps[0] == nightfall::toolbox::kTrapTickCount);
  CHECK(result.unimplemented_traps.empty());
  CHECK(result.final_pc >= 0x00010004u);
}

TEST_CASE("runtime records unimplemented A-line trap") {
  const uint8_t program[] = {
      0xA0u,
      0x00u,
      0x4Eu,
      0x75u,
  };
  nightfall::m68k::Runtime runtime;
  CHECK(runtime.load_program(program, sizeof(program)) == NF_OK);

  const nightfall::m68k::RuntimeResult result = runtime.run();
  CHECK(result.status == NF_ERROR_UNIMPLEMENTED);
  CHECK(std::strcmp(result.stop_reason, "unimplemented-trap") == 0);
  REQUIRE(result.traps.size() == 1u);
  CHECK(result.traps[0] == 0xA000u);
  REQUIRE(result.unimplemented_traps.size() == 1u);
  CHECK(result.unimplemented_traps[0] == 0xA000u);
}
