#include "ToolboxManagers.h"

namespace nightfall::toolbox {

#define NF_TOOLBOX_TRAP(trap_word) record_trace(trap_word)

namespace {

bool is_nil(uint32_t address) {
  return address == 0u;
}

}  // namespace

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::new_handle(uint32_t logical_size, uint32_t *out_handle) {
  NF_TOOLBOX_TRAP(kTrapNewHandle);

  if (out_handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (MemoryHandle &handle : handles_) {
    if (!handle.allocated) {
      handle = MemoryHandle{
          true,
          next_handle_address_,
          next_handle_data_address_,
          logical_size,
          0u,
      };
      *out_handle = handle.address;
      next_handle_address_ += 4u;
      next_handle_data_address_ += logical_size == 0u ? 4u : logical_size;
      return NF_OK;
    }
  }

  return NF_ERROR_RESOURCE_EXHAUSTED;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::dispose_handle(uint32_t handle_address) {
  NF_TOOLBOX_TRAP(kTrapDisposeHandle);

  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  if ((handle->state & kHandleResourceBit) != 0u) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->allocated = false;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::get_handle_size(uint32_t handle_address, uint32_t *out_size) const {
  NF_TOOLBOX_TRAP(kTrapGetHandleSize);

  if (out_size == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  const MemoryHandle *handle = find_handle(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_size = handle->logical_size;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::set_handle_size(uint32_t handle_address, uint32_t logical_size) {
  NF_TOOLBOX_TRAP(kTrapSetHandleSize);

  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->logical_size = logical_size;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::h_lock(uint32_t handle_address) {
  NF_TOOLBOX_TRAP(kTrapHLock);

  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->state = static_cast<uint8_t>(handle->state | kHandleLockedBit);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::h_unlock(uint32_t handle_address) {
  NF_TOOLBOX_TRAP(kTrapHUnlock);

  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->state = static_cast<uint8_t>(handle->state & ~kHandleLockedBit);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::h_get_state(uint32_t handle_address, uint8_t *out_state) const {
  NF_TOOLBOX_TRAP(kTrapHGetState);

  if (out_state == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  const MemoryHandle *handle = find_handle(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_state = handle->state;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::h_set_state(uint32_t handle_address, uint8_t state) {
  NF_TOOLBOX_TRAP(kTrapHSetState);

  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->state = state;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::new_ptr(uint32_t logical_size, uint32_t *out_pointer) {
  NF_TOOLBOX_TRAP(kTrapNewPtr);

  if (out_pointer == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (MemoryPointer &pointer : pointers_) {
    if (!pointer.allocated) {
      pointer = MemoryPointer{true, next_pointer_address_, logical_size};
      *out_pointer = pointer.address;
      next_pointer_address_ += logical_size == 0u ? 4u : logical_size;
      return NF_OK;
    }
  }

  return NF_ERROR_RESOURCE_EXHAUSTED;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::dispose_ptr(uint32_t pointer_address) {
  NF_TOOLBOX_TRAP(kTrapDisposePtr);

  MemoryPointer *pointer = find_pointer_mutable(pointer_address);
  if (pointer == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  pointer->allocated = false;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::add_resource(uint32_t type, int16_t id, uint32_t logical_size, uint32_t *out_handle) {
  if (out_handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  if (find_resource(type, id) != nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (ResourceEntry &resource : resources_) {
    if (!resource.present) {
      resource = ResourceEntry{true, false, false, type, id, logical_size, 0u};
      return get_resource(type, id, out_handle);
    }
  }

  return NF_ERROR_RESOURCE_EXHAUSTED;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::get_resource(uint32_t type, int16_t id, uint32_t *out_handle) {
  NF_TOOLBOX_TRAP(kTrapGetResource);

  if (out_handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  ResourceEntry *resource = find_resource_mutable(type, id);
  if (resource == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  if (resource->loaded && !resource->detached) {
    *out_handle = resource->handle_address;
    return NF_OK;
  }

  uint32_t handle_address = 0u;
  const nf_status status = new_handle(resource->logical_size, &handle_address);
  if (status != NF_OK) {
    return status;
  }

  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (handle == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->state = static_cast<uint8_t>(handle->state | kHandleResourceBit);
  resource->loaded = true;
  resource->detached = false;
  resource->handle_address = handle_address;
  *out_handle = handle_address;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::release_resource(uint32_t handle_address) {
  NF_TOOLBOX_TRAP(kTrapReleaseResource);

  ResourceEntry *resource = find_resource_for_handle_mutable(handle_address);
  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (resource == nullptr || handle == nullptr || resource->detached) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->allocated = false;
  resource->loaded = false;
  resource->handle_address = 0u;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::get_resource_size_on_disk(uint32_t handle_address, uint32_t *out_size) const {
  NF_TOOLBOX_TRAP(kTrapGetResourceSizeOnDisk);

  if (out_size == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  const ResourceEntry *resource = find_resource_for_handle(handle_address);
  if (resource == nullptr || resource->detached) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_size = resource->logical_size;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::detach_resource(uint32_t handle_address) {
  NF_TOOLBOX_TRAP(kTrapDetachResource);

  ResourceEntry *resource = find_resource_for_handle_mutable(handle_address);
  MemoryHandle *handle = find_handle_mutable(handle_address);
  if (resource == nullptr || handle == nullptr || resource->detached) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  handle->state = static_cast<uint8_t>(handle->state & ~kHandleResourceBit);
  resource->detached = true;
  resource->loaded = false;
  resource->handle_address = 0u;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::tick_count(uint32_t *out_ticks) const {
  NF_TOOLBOX_TRAP(kTrapTickCount);

  if (out_ticks == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_ticks = static_cast<uint32_t>(ticks_ & 0xFFFFFFFFu);
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::lm_get_ticks(uint32_t *out_ticks) const {
  return tick_count(out_ticks);
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::delay(uint32_t ticks, uint32_t *out_final_ticks) {
  NF_TOOLBOX_TRAP(kTrapDelay);

  ticks_ += ticks;
  if (out_final_ticks != nullptr) {
    *out_final_ticks = static_cast<uint32_t>(ticks_ & 0xFFFFFFFFu);
  }
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::get_date_time(uint32_t *out_seconds) const {
  NF_TOOLBOX_TRAP(kTrapGetDateTime);

  if (out_seconds == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_seconds = date_time_seconds_;
  return NF_OK;
}

void ManagerState::set_ticks(uint64_t ticks) {
  ticks_ = ticks;
}

void ManagerState::advance_ticks(uint32_t ticks) {
  ticks_ += ticks;
}

void ManagerState::set_date_time(uint32_t seconds) {
  date_time_seconds_ = seconds;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::snd_new_channel(uint32_t *out_channel) {
  NF_TOOLBOX_TRAP(kTrapSndNewChannel);

  if (out_channel == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  for (SoundChannel &channel : sound_channels_) {
    if (!channel.allocated) {
      channel = SoundChannel{true, next_sound_channel_address_, 0u, 0u, 0u};
      *out_channel = channel.address;
      next_sound_channel_address_ += 0x100u;
      return NF_OK;
    }
  }

  return NF_ERROR_RESOURCE_EXHAUSTED;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::snd_do_command(uint32_t channel_address, uint16_t command) {
  NF_TOOLBOX_TRAP(kTrapSndDoCommand);

  SoundChannel *channel = find_sound_channel_mutable(channel_address);
  if (channel == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  ++channel->queued_commands;
  channel->last_command = command;
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Toolbox Managers".
nf_status ManagerState::snd_play(uint32_t channel_address, uint32_t sound_handle) {
  NF_TOOLBOX_TRAP(kTrapSndPlay);

  SoundChannel *channel = find_sound_channel_mutable(channel_address);
  if (channel == nullptr || find_handle(sound_handle) == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  ++channel->queued_commands;
  channel->last_sound_handle = sound_handle;
  return NF_OK;
}

nf_status ManagerState::dispatch(uint16_t trap_word, uint32_t argument) {
  switch (trap_word) {
    case kTrapNewHandle: {
      uint32_t ignored = 0u;
      return new_handle(argument, &ignored);
    }
    case kTrapDisposeHandle:
      return dispose_handle(argument);
    case kTrapGetHandleSize: {
      uint32_t ignored = 0u;
      return get_handle_size(argument, &ignored);
    }
    case kTrapSetHandleSize:
      return set_handle_size(argument, 0u);
    case kTrapHLock:
      return h_lock(argument);
    case kTrapHUnlock:
      return h_unlock(argument);
    case kTrapHGetState: {
      uint8_t ignored = 0u;
      return h_get_state(argument, &ignored);
    }
    case kTrapHSetState:
      return h_set_state(argument, 0u);
    case kTrapNewPtr: {
      uint32_t ignored = 0u;
      return new_ptr(argument, &ignored);
    }
    case kTrapDisposePtr:
      return dispose_ptr(argument);
    case kTrapReleaseResource:
      return release_resource(argument);
    case kTrapGetResourceSizeOnDisk: {
      uint32_t ignored = 0u;
      return get_resource_size_on_disk(argument, &ignored);
    }
    case kTrapDetachResource:
      return detach_resource(argument);
    case kTrapTickCount: {
      uint32_t ignored = 0u;
      return tick_count(&ignored);
    }
    case kTrapGetDateTime: {
      uint32_t ignored = 0u;
      return get_date_time(&ignored);
    }
    case kTrapDelay:
      return delay(argument, nullptr);
    default:
      return NF_ERROR_UNIMPLEMENTED;
  }
}

const MemoryHandle *ManagerState::find_handle(uint32_t handle_address) const {
  if (is_nil(handle_address)) {
    return nullptr;
  }

  for (const MemoryHandle &handle : handles_) {
    if (handle.allocated && handle.address == handle_address) {
      return &handle;
    }
  }
  return nullptr;
}

const MemoryPointer *ManagerState::find_pointer(uint32_t pointer_address) const {
  if (is_nil(pointer_address)) {
    return nullptr;
  }

  for (const MemoryPointer &pointer : pointers_) {
    if (pointer.allocated && pointer.address == pointer_address) {
      return &pointer;
    }
  }
  return nullptr;
}

const ResourceEntry *ManagerState::find_resource(uint32_t type, int16_t id) const {
  for (const ResourceEntry &resource : resources_) {
    if (resource.present && resource.type == type && resource.id == id) {
      return &resource;
    }
  }
  return nullptr;
}

const SoundChannel *ManagerState::find_sound_channel(uint32_t channel_address) const {
  if (is_nil(channel_address)) {
    return nullptr;
  }

  for (const SoundChannel &channel : sound_channels_) {
    if (channel.allocated && channel.address == channel_address) {
      return &channel;
    }
  }
  return nullptr;
}

size_t ManagerState::trace_count() const {
  return trace_count_;
}

uint16_t ManagerState::trace_word(size_t index) const {
  return index < trace_count_ ? trace_words_[index] : 0u;
}

MemoryHandle *ManagerState::find_handle_mutable(uint32_t handle_address) {
  return const_cast<MemoryHandle *>(static_cast<const ManagerState *>(this)->find_handle(handle_address));
}

MemoryPointer *ManagerState::find_pointer_mutable(uint32_t pointer_address) {
  return const_cast<MemoryPointer *>(static_cast<const ManagerState *>(this)->find_pointer(pointer_address));
}

ResourceEntry *ManagerState::find_resource_mutable(uint32_t type, int16_t id) {
  return const_cast<ResourceEntry *>(static_cast<const ManagerState *>(this)->find_resource(type, id));
}

ResourceEntry *ManagerState::find_resource_for_handle_mutable(uint32_t handle_address) {
  return const_cast<ResourceEntry *>(static_cast<const ManagerState *>(this)->find_resource_for_handle(handle_address));
}

const ResourceEntry *ManagerState::find_resource_for_handle(uint32_t handle_address) const {
  if (is_nil(handle_address)) {
    return nullptr;
  }

  for (const ResourceEntry &resource : resources_) {
    if (resource.present && resource.loaded && resource.handle_address == handle_address) {
      return &resource;
    }
  }
  return nullptr;
}

SoundChannel *ManagerState::find_sound_channel_mutable(uint32_t channel_address) {
  return const_cast<SoundChannel *>(static_cast<const ManagerState *>(this)->find_sound_channel(channel_address));
}

void ManagerState::record_trace(uint16_t trap_word) const {
  if (trace_count_ < kMaxTraceWords) {
    trace_words_[trace_count_++] = trap_word;
  }
}

}  // namespace nightfall::toolbox
