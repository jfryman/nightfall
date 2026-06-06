#include "doctest.h"

#include "ToolboxManagers.h"

using nightfall::toolbox::ManagerState;

TEST_CASE("Memory Manager models handle allocation, resizing, and disposal") {
  ManagerState state;

  uint32_t handle = 0u;
  CHECK(state.new_handle(64u, &handle) == NF_OK);
  CHECK(handle != 0u);

  const auto *allocated = state.find_handle(handle);
  REQUIRE(allocated != nullptr);
  CHECK(allocated->logical_size == 64u);
  CHECK(allocated->state == 0u);
  CHECK(allocated->data_address != 0u);

  uint32_t size = 0u;
  CHECK(state.get_handle_size(handle, &size) == NF_OK);
  CHECK(size == 64u);
  CHECK(state.set_handle_size(handle, 96u) == NF_OK);
  CHECK(state.get_handle_size(handle, &size) == NF_OK);
  CHECK(size == 96u);

  CHECK(state.dispose_handle(handle) == NF_OK);
  CHECK(state.find_handle(handle) == nullptr);
  CHECK(state.get_handle_size(handle, &size) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.dispose_handle(handle) == NF_ERROR_INVALID_ARGUMENT);

  REQUIRE(state.trace_count() >= 5u);
  CHECK(state.trace_word(0u) == nightfall::toolbox::kTrapNewHandle);
  CHECK(state.trace_word(1u) == nightfall::toolbox::kTrapGetHandleSize);
  CHECK(state.trace_word(2u) == nightfall::toolbox::kTrapSetHandleSize);
}

TEST_CASE("Memory Manager preserves handle state flags") {
  ManagerState state;

  uint32_t handle = 0u;
  REQUIRE(state.new_handle(32u, &handle) == NF_OK);

  uint8_t handle_state = 0xFFu;
  CHECK(state.h_get_state(handle, &handle_state) == NF_OK);
  CHECK(handle_state == 0u);

  CHECK(state.h_lock(handle) == NF_OK);
  CHECK(state.h_get_state(handle, &handle_state) == NF_OK);
  CHECK((handle_state & nightfall::toolbox::kHandleLockedBit) != 0u);

  CHECK(state.h_unlock(handle) == NF_OK);
  CHECK(state.h_get_state(handle, &handle_state) == NF_OK);
  CHECK((handle_state & nightfall::toolbox::kHandleLockedBit) == 0u);

  CHECK(state.h_set_state(handle, nightfall::toolbox::kHandleLockedBit | nightfall::toolbox::kHandlePurgeableBit) == NF_OK);
  CHECK(state.h_get_state(handle, &handle_state) == NF_OK);
  CHECK(handle_state == (nightfall::toolbox::kHandleLockedBit | nightfall::toolbox::kHandlePurgeableBit));
  CHECK(state.h_get_state(0x0000BAD0u, &handle_state) == NF_ERROR_INVALID_ARGUMENT);
}

TEST_CASE("Memory Manager models nonrelocatable pointer lifecycle") {
  ManagerState state;

  uint32_t pointer = 0u;
  CHECK(state.new_ptr(48u, &pointer) == NF_OK);
  CHECK(pointer != 0u);

  const auto *allocated = state.find_pointer(pointer);
  REQUIRE(allocated != nullptr);
  CHECK(allocated->logical_size == 48u);

  CHECK(state.dispose_ptr(pointer) == NF_OK);
  CHECK(state.find_pointer(pointer) == nullptr);
  CHECK(state.dispose_ptr(pointer) == NF_ERROR_INVALID_ARGUMENT);

  REQUIRE(state.trace_count() >= 2u);
  CHECK(state.trace_word(0u) == nightfall::toolbox::kTrapNewPtr);
  CHECK(state.trace_word(1u) == nightfall::toolbox::kTrapDisposePtr);
}

TEST_CASE("Resource Manager loads, sizes, releases, and reloads resources") {
  ManagerState state;

  uint32_t handle = 0u;
  CHECK(state.add_resource(nightfall::toolbox::kResourceTypeSnd, 128, 512u, &handle) == NF_OK);
  CHECK(handle != 0u);

  const auto *resource = state.find_resource(nightfall::toolbox::kResourceTypeSnd, 128);
  REQUIRE(resource != nullptr);
  CHECK(resource->loaded == true);
  CHECK(resource->handle_address == handle);

  const auto *memory_handle = state.find_handle(handle);
  REQUIRE(memory_handle != nullptr);
  CHECK((memory_handle->state & nightfall::toolbox::kHandleResourceBit) != 0u);

  uint32_t size = 0u;
  CHECK(state.get_resource_size_on_disk(handle, &size) == NF_OK);
  CHECK(size == 512u);

  uint32_t same_handle = 0u;
  CHECK(state.get_resource(nightfall::toolbox::kResourceTypeSnd, 128, &same_handle) == NF_OK);
  CHECK(same_handle == handle);

  CHECK(state.release_resource(handle) == NF_OK);
  CHECK(state.find_handle(handle) == nullptr);
  CHECK(state.get_resource_size_on_disk(handle, &size) == NF_ERROR_INVALID_ARGUMENT);

  uint32_t reloaded_handle = 0u;
  CHECK(state.get_resource(nightfall::toolbox::kResourceTypeSnd, 128, &reloaded_handle) == NF_OK);
  CHECK(reloaded_handle != 0u);
  CHECK(reloaded_handle != handle);
}

TEST_CASE("Resource Manager detach leaves application handle usable but unowned") {
  ManagerState state;

  uint32_t handle = 0u;
  REQUIRE(state.add_resource(nightfall::toolbox::kResourceTypeSnd, 7, 44u, &handle) == NF_OK);
  CHECK(state.detach_resource(handle) == NF_OK);

  const auto *detached_handle = state.find_handle(handle);
  REQUIRE(detached_handle != nullptr);
  CHECK((detached_handle->state & nightfall::toolbox::kHandleResourceBit) == 0u);

  const auto *resource = state.find_resource(nightfall::toolbox::kResourceTypeSnd, 7);
  REQUIRE(resource != nullptr);
  CHECK(resource->detached == true);
  CHECK(resource->loaded == false);
  CHECK(resource->handle_address == 0u);

  CHECK(state.release_resource(handle) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.dispose_handle(handle) == NF_OK);
}

TEST_CASE("Virtual time routines are deterministic and wrap to 32-bit ticks") {
  ManagerState state;
  state.set_ticks(0x00000001FFFFFFFFull);
  state.set_date_time(0x12345678u);

  uint32_t ticks = 0u;
  CHECK(state.tick_count(&ticks) == NF_OK);
  CHECK(ticks == 0xFFFFFFFFu);
  CHECK(state.lm_get_ticks(&ticks) == NF_OK);
  CHECK(ticks == 0xFFFFFFFFu);

  CHECK(state.delay(2u, &ticks) == NF_OK);
  CHECK(ticks == 1u);
  CHECK(state.tick_count(nullptr) == NF_ERROR_INVALID_ARGUMENT);

  uint32_t date_time = 0u;
  CHECK(state.get_date_time(&date_time) == NF_OK);
  CHECK(date_time == 0x12345678u);
  CHECK(state.get_date_time(nullptr) == NF_ERROR_INVALID_ARGUMENT);
}

TEST_CASE("Silent Sound Manager records channels and commands without audio output") {
  ManagerState state;

  uint32_t sound_handle = 0u;
  REQUIRE(state.add_resource(nightfall::toolbox::kResourceTypeSnd, 4, 128u, &sound_handle) == NF_OK);

  uint32_t channel = 0u;
  CHECK(state.snd_new_channel(&channel) == NF_OK);
  CHECK(channel != 0u);

  CHECK(state.snd_do_command(channel, 0x0051u) == NF_OK);
  CHECK(state.snd_play(channel, sound_handle) == NF_OK);

  const auto *sound_channel = state.find_sound_channel(channel);
  REQUIRE(sound_channel != nullptr);
  CHECK(sound_channel->queued_commands == 2u);
  CHECK(sound_channel->last_command == 0x0051u);
  CHECK(sound_channel->last_sound_handle == sound_handle);
  CHECK(state.snd_do_command(0x00BADBADu, 0x0001u) == NF_ERROR_INVALID_ARGUMENT);
  CHECK(state.snd_play(channel, 0x00BADBADu) == NF_ERROR_INVALID_ARGUMENT);
}

TEST_CASE("Phase 5 dispatcher traces supported Toolbox trap words") {
  ManagerState state;

  CHECK(state.dispatch(nightfall::toolbox::kTrapNewHandle, 24u) == NF_OK);
  const uint32_t handle = 0x00020000u;
  CHECK(state.dispatch(nightfall::toolbox::kTrapHLock, handle) == NF_OK);
  CHECK(state.dispatch(nightfall::toolbox::kTrapTickCount, 0u) == NF_OK);
  CHECK(state.dispatch(0xA8F6u, 0u) == NF_ERROR_UNIMPLEMENTED);

  const auto *memory_handle = state.find_handle(handle);
  REQUIRE(memory_handle != nullptr);
  CHECK((memory_handle->state & nightfall::toolbox::kHandleLockedBit) != 0u);
  REQUIRE(state.trace_count() >= 3u);
  CHECK(state.trace_word(0u) == nightfall::toolbox::kTrapNewHandle);
  CHECK(state.trace_word(1u) == nightfall::toolbox::kTrapHLock);
  CHECK(state.trace_word(2u) == nightfall::toolbox::kTrapTickCount);
}
