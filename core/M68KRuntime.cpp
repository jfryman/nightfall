#include "M68KRuntime.h"

#include "QuickDrawPort.h"
#include "ToolboxManagers.h"

#include <stddef.h>

#include <algorithm>

extern "C" {
#include "m68k.h"
}

namespace nightfall::m68k {

namespace {

constexpr uint16_t kALineMask = 0xF000u;
constexpr uint16_t kALinePrefix = 0xA000u;
constexpr uint16_t kNopOpcode = 0x4E71u;
constexpr uint16_t kRtsOpcode = 0x4E75u;
constexpr uint16_t kIllegalOpcode = 0x4AFCu;

Runtime *g_active_runtime = nullptr;

bool is_aline(uint16_t word) {
  return (word & kALineMask) == kALinePrefix;
}

}  // namespace

Runtime::Runtime(RuntimeOptions options) : options_(options), memory_(options.memory_size, 0u) {
  result_.status = NF_ERROR_INCOMPLETE_PROGRAM;
  result_.final_pc = options_.entry_address;
  result_.cycles_run = 0u;
  result_.stop_reason = "not-run";
}

nf_status Runtime::load_program(const uint8_t *bytes, size_t byte_count) {
  if (bytes == nullptr || byte_count == 0u || !valid_address(options_.code_address, byte_count)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  std::fill(memory_.begin(), memory_.end(), 0u);
  std::copy(bytes, bytes + byte_count, memory_.begin() + options_.code_address);
  write_long(0u, options_.stack_address);
  write_long(4u, options_.entry_address);
  loaded_ = true;
  return NF_OK;
}

RuntimeResult Runtime::run() {
  result_ = RuntimeResult{
      NF_ERROR_INCOMPLETE_PROGRAM,
      options_.entry_address,
      0u,
      "cycle-budget",
      {},
      {},
  };
  stopped_ = false;

  if (!loaded_) {
    result_.status = NF_ERROR_INVALID_ARGUMENT;
    result_.stop_reason = "no-program";
    return result_;
  }

  g_active_runtime = this;
  m68k_init();
  m68k_set_cpu_type(M68K_CPU_TYPE_68000);
  m68k_pulse_reset();
  m68k_set_reg(M68K_REG_PC, options_.entry_address);
  m68k_set_reg(M68K_REG_SP, options_.stack_address);

  result_.cycles_run = static_cast<uint32_t>(m68k_execute(static_cast<int>(options_.cycle_budget)));
  result_.final_pc = m68k_get_reg(nullptr, M68K_REG_PC);

  if (!stopped_) {
    result_.status = NF_ERROR_INCOMPLETE_PROGRAM;
    result_.stop_reason = "cycle-budget";
  }

  g_active_runtime = nullptr;
  return result_;
}

uint8_t Runtime::read_byte(uint32_t address) const {
  if (!valid_address(address, 1u)) {
    return 0u;
  }
  return memory_[address];
}

uint16_t Runtime::read_word(uint32_t address) const {
  if (!valid_address(address, 2u)) {
    return 0u;
  }
  return static_cast<uint16_t>((static_cast<uint16_t>(memory_[address]) << 8u) | memory_[address + 1u]);
}

void Runtime::write_byte(uint32_t address, uint8_t value) {
  if (valid_address(address, 1u)) {
    memory_[address] = value;
  }
}

bool Runtime::valid_address(uint32_t address, size_t length) const {
  return static_cast<size_t>(address) <= memory_.size() && length <= memory_.size() - static_cast<size_t>(address);
}

void Runtime::write_word(uint32_t address, uint16_t value) {
  if (!valid_address(address, 2u)) {
    return;
  }
  memory_[address] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
  memory_[address + 1u] = static_cast<uint8_t>(value & 0xFFu);
}

void Runtime::write_long(uint32_t address, uint32_t value) {
  if (!valid_address(address, 4u)) {
    return;
  }
  memory_[address] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
  memory_[address + 1u] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
  memory_[address + 2u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
  memory_[address + 3u] = static_cast<uint8_t>(value & 0xFFu);
}

// Source: docs/clean-room-sources.md, "Phase 5 Resource Fork Runner Backfill".
void Runtime::on_instruction(uint32_t pc) {
  const uint16_t word = read_word(pc);
  if (word == kRtsOpcode) {
    result_.status = NF_OK;
    result_.stop_reason = "rts";
    stopped_ = true;
    m68k_set_reg(M68K_REG_PC, pc + 2u);
    m68k_end_timeslice();
    return;
  }

  if (word == kIllegalOpcode) {
    result_.status = NF_ERROR_UNSUPPORTED_INSTRUCTION;
    result_.stop_reason = "illegal";
    stopped_ = true;
    m68k_end_timeslice();
    return;
  }

  if (!is_aline(word)) {
    return;
  }

  const bool implemented = handle_trap(word);
  write_word(pc, kNopOpcode);
  if (!implemented && options_.stop_on_unimplemented) {
    result_.status = NF_ERROR_UNIMPLEMENTED;
    result_.stop_reason = "unimplemented-trap";
    stopped_ = true;
    m68k_end_timeslice();
  }
}

bool Runtime::handle_trap(uint16_t trap_word) {
  result_.traps.push_back(trap_word);
  if (!is_implemented_trap(trap_word)) {
    result_.unimplemented_traps.push_back(trap_word);
    return false;
  }
  return true;
}

bool Runtime::is_implemented_trap(uint16_t trap_word) const {
  using namespace nightfall::quickdraw;
  using namespace nightfall::toolbox;

  switch (trap_word) {
    case kTrapInitPort:
    case kTrapInitGraf:
    case kTrapOpenPort:
    case kTrapForeColor:
    case kTrapBackColor:
    case kTrapSetPort:
    case kTrapGetPort:
    case kTrapSetPortBits:
    case kTrapSetClip:
    case kTrapGetClip:
    case kTrapClosePort:
    case kTrapFrameRect:
    case kTrapPaintRect:
    case kTrapEraseRect:
    case kTrapInvertRect:
    case kTrapFillRect:
    case kTrapCopyMask:
    case kTrapLineTo:
    case kTrapLine:
    case kTrapMoveTo:
    case kTrapMove:
    case kTrapRGBForeColor:
    case kTrapRGBBackColor:
    case kTrapNewRgn:
    case kTrapDisposeRgn:
    case kTrapCopyRgn:
    case kTrapSetRectRgn:
    case kTrapRectRgn:
    case kTrapOffsetRgn:
    case kTrapEmptyRgn:
    case kTrapEqualRgn:
    case kTrapSectRgn:
    case kTrapUnionRgn:
    case kTrapDiffRgn:
    case kTrapPtInRgn:
    case kTrapCopyBits:
    case kTrapDrawPicture:
    case kTrapNewPtr:
    case kTrapDisposePtr:
    case kTrapNewHandle:
    case kTrapDisposeHandle:
    case kTrapSetHandleSize:
    case kTrapGetHandleSize:
    case kTrapHLock:
    case kTrapHUnlock:
    case kTrapHGetState:
    case kTrapHSetState:
    case kTrapDetachResource:
    case kTrapGetResource:
    case kTrapReleaseResource:
    case kTrapGetResourceSizeOnDisk:
    case kTrapGetDateTime:
    case kTrapDelay:
    case kTrapTickCount:
    case kTrapSndDoCommand:
    case kTrapSndPlay:
    case kTrapSndNewChannel:
      return true;
    default:
      return false;
  }
}

}  // namespace nightfall::m68k

extern "C" void nightfall_m68k_instruction_hook(unsigned int pc) {
  if (nightfall::m68k::g_active_runtime != nullptr) {
    nightfall::m68k::g_active_runtime->on_instruction(pc);
  }
}

extern "C" unsigned int m68k_read_memory_8(unsigned int address) {
  if (nightfall::m68k::g_active_runtime == nullptr) {
    return 0u;
  }
  return nightfall::m68k::g_active_runtime->read_byte(address);
}

extern "C" unsigned int m68k_read_memory_16(unsigned int address) {
  if (nightfall::m68k::g_active_runtime == nullptr) {
    return 0u;
  }
  return nightfall::m68k::g_active_runtime->read_word(address);
}

extern "C" unsigned int m68k_read_memory_32(unsigned int address) {
  if (nightfall::m68k::g_active_runtime == nullptr) {
    return 0u;
  }
  const uint16_t hi = nightfall::m68k::g_active_runtime->read_word(address);
  const uint16_t lo = nightfall::m68k::g_active_runtime->read_word(address + 2u);
  return (static_cast<unsigned int>(hi) << 16u) | lo;
}

extern "C" void m68k_write_memory_8(unsigned int address, unsigned int value) {
  if (nightfall::m68k::g_active_runtime != nullptr) {
    nightfall::m68k::g_active_runtime->write_byte(address, static_cast<uint8_t>(value & 0xFFu));
  }
}

extern "C" void m68k_write_memory_16(unsigned int address, unsigned int value) {
  if (nightfall::m68k::g_active_runtime != nullptr) {
    nightfall::m68k::g_active_runtime->write_byte(address, static_cast<uint8_t>((value >> 8u) & 0xFFu));
    nightfall::m68k::g_active_runtime->write_byte(address + 1u, static_cast<uint8_t>(value & 0xFFu));
  }
}

extern "C" void m68k_write_memory_32(unsigned int address, unsigned int value) {
  if (nightfall::m68k::g_active_runtime != nullptr) {
    nightfall::m68k::g_active_runtime->write_byte(address, static_cast<uint8_t>((value >> 24u) & 0xFFu));
    nightfall::m68k::g_active_runtime->write_byte(address + 1u, static_cast<uint8_t>((value >> 16u) & 0xFFu));
    nightfall::m68k::g_active_runtime->write_byte(address + 2u, static_cast<uint8_t>((value >> 8u) & 0xFFu));
    nightfall::m68k::g_active_runtime->write_byte(address + 3u, static_cast<uint8_t>(value & 0xFFu));
  }
}
