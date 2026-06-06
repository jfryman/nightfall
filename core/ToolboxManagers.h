#ifndef NIGHTFALL_CORE_TOOLBOXMANAGERS_H
#define NIGHTFALL_CORE_TOOLBOXMANAGERS_H

#include "nightfall.h"

#include <stddef.h>
#include <stdint.h>

namespace nightfall::toolbox {

constexpr uint16_t kTrapNewPtr = 0xA11Eu;
constexpr uint16_t kTrapDisposePtr = 0xA01Fu;
constexpr uint16_t kTrapNewHandle = 0xA122u;
constexpr uint16_t kTrapDisposeHandle = 0xA023u;
constexpr uint16_t kTrapSetHandleSize = 0xA024u;
constexpr uint16_t kTrapGetHandleSize = 0xA025u;
constexpr uint16_t kTrapHLock = 0xA029u;
constexpr uint16_t kTrapHUnlock = 0xA02Au;
constexpr uint16_t kTrapHGetState = 0xA069u;
constexpr uint16_t kTrapHSetState = 0xA06Au;
constexpr uint16_t kTrapDetachResource = 0xA992u;
constexpr uint16_t kTrapGetResource = 0xA9A0u;
constexpr uint16_t kTrapReleaseResource = 0xA9A3u;
constexpr uint16_t kTrapGetResourceSizeOnDisk = 0xA9A5u;
constexpr uint16_t kTrapGetDateTime = 0xA039u;
constexpr uint16_t kTrapDelay = 0xA03Bu;
constexpr uint16_t kTrapTickCount = 0xA975u;
constexpr uint16_t kTrapSndDoCommand = 0xA803u;
constexpr uint16_t kTrapSndPlay = 0xA805u;
constexpr uint16_t kTrapSndNewChannel = 0xA807u;

constexpr uint8_t kHandleResourceBit = 0x20u;
constexpr uint8_t kHandlePurgeableBit = 0x40u;
constexpr uint8_t kHandleLockedBit = 0x80u;
constexpr uint32_t kResourceTypeSnd = 0x736E6420u;

struct MemoryHandle {
  bool allocated;
  uint32_t address;
  uint32_t data_address;
  uint32_t logical_size;
  uint8_t state;
};

struct MemoryPointer {
  bool allocated;
  uint32_t address;
  uint32_t logical_size;
};

struct ResourceEntry {
  bool present;
  bool loaded;
  bool detached;
  uint32_t type;
  int16_t id;
  uint32_t logical_size;
  uint32_t handle_address;
};

struct SoundChannel {
  bool allocated;
  uint32_t address;
  uint16_t queued_commands;
  uint16_t last_command;
  uint32_t last_sound_handle;
};

class ManagerState {
 public:
  nf_status new_handle(uint32_t logical_size, uint32_t *out_handle);
  nf_status dispose_handle(uint32_t handle_address);
  nf_status get_handle_size(uint32_t handle_address, uint32_t *out_size) const;
  nf_status set_handle_size(uint32_t handle_address, uint32_t logical_size);
  nf_status h_lock(uint32_t handle_address);
  nf_status h_unlock(uint32_t handle_address);
  nf_status h_get_state(uint32_t handle_address, uint8_t *out_state) const;
  nf_status h_set_state(uint32_t handle_address, uint8_t state);

  nf_status new_ptr(uint32_t logical_size, uint32_t *out_pointer);
  nf_status dispose_ptr(uint32_t pointer_address);

  nf_status add_resource(uint32_t type, int16_t id, uint32_t logical_size, uint32_t *out_handle);
  nf_status get_resource(uint32_t type, int16_t id, uint32_t *out_handle);
  nf_status release_resource(uint32_t handle_address);
  nf_status get_resource_size_on_disk(uint32_t handle_address, uint32_t *out_size) const;
  nf_status detach_resource(uint32_t handle_address);

  nf_status tick_count(uint32_t *out_ticks) const;
  nf_status lm_get_ticks(uint32_t *out_ticks) const;
  nf_status delay(uint32_t ticks, uint32_t *out_final_ticks);
  nf_status get_date_time(uint32_t *out_seconds) const;
  void set_ticks(uint64_t ticks);
  void advance_ticks(uint32_t ticks);
  void set_date_time(uint32_t seconds);

  nf_status snd_new_channel(uint32_t *out_channel);
  nf_status snd_do_command(uint32_t channel_address, uint16_t command);
  nf_status snd_play(uint32_t channel_address, uint32_t sound_handle);

  nf_status dispatch(uint16_t trap_word, uint32_t argument);

  const MemoryHandle *find_handle(uint32_t handle_address) const;
  const MemoryPointer *find_pointer(uint32_t pointer_address) const;
  const ResourceEntry *find_resource(uint32_t type, int16_t id) const;
  const SoundChannel *find_sound_channel(uint32_t channel_address) const;
  size_t trace_count() const;
  uint16_t trace_word(size_t index) const;

 private:
  static constexpr size_t kMaxHandles = 32u;
  static constexpr size_t kMaxPointers = 32u;
  static constexpr size_t kMaxResources = 32u;
  static constexpr size_t kMaxSoundChannels = 8u;
  static constexpr size_t kMaxTraceWords = 64u;

  MemoryHandle *find_handle_mutable(uint32_t handle_address);
  MemoryPointer *find_pointer_mutable(uint32_t pointer_address);
  ResourceEntry *find_resource_mutable(uint32_t type, int16_t id);
  ResourceEntry *find_resource_for_handle_mutable(uint32_t handle_address);
  const ResourceEntry *find_resource_for_handle(uint32_t handle_address) const;
  SoundChannel *find_sound_channel_mutable(uint32_t channel_address);
  void record_trace(uint16_t trap_word) const;

  MemoryHandle handles_[kMaxHandles]{};
  MemoryPointer pointers_[kMaxPointers]{};
  ResourceEntry resources_[kMaxResources]{};
  SoundChannel sound_channels_[kMaxSoundChannels]{};
  uint32_t next_handle_address_ = 0x00020000u;
  uint32_t next_handle_data_address_ = 0x00030000u;
  uint32_t next_pointer_address_ = 0x00040000u;
  uint32_t next_sound_channel_address_ = 0x00050000u;
  uint64_t ticks_ = 0u;
  uint32_t date_time_seconds_ = 0u;
  mutable uint16_t trace_words_[kMaxTraceWords]{};
  mutable size_t trace_count_ = 0u;
};

}  // namespace nightfall::toolbox

#endif
