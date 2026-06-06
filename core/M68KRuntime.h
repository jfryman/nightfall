#ifndef NIGHTFALL_CORE_M68KRUNTIME_H
#define NIGHTFALL_CORE_M68KRUNTIME_H

#include "nightfall.h"

#include <stdint.h>

#include <vector>

extern "C" void nightfall_m68k_instruction_hook(unsigned int pc);

namespace nightfall::m68k {

struct RuntimeOptions {
  uint32_t code_address = 0x00010000u;
  uint32_t entry_address = 0x00010000u;
  uint32_t stack_address = 0x00080000u;
  size_t memory_size = 0x00100000u;
  uint32_t cycle_budget = 1000000u;
  bool stop_on_unimplemented = true;
};

struct RuntimeResult {
  nf_status status;
  uint32_t final_pc;
  uint32_t cycles_run;
  const char *stop_reason;
  std::vector<uint16_t> traps;
  std::vector<uint16_t> unimplemented_traps;
};

class Runtime {
 public:
  explicit Runtime(RuntimeOptions options = RuntimeOptions{});

  nf_status load_program(const uint8_t *bytes, size_t byte_count);
  RuntimeResult run();
  uint16_t read_word(uint32_t address) const;
  void write_byte(uint32_t address, uint8_t value);
  uint8_t read_byte(uint32_t address) const;

 private:
  friend void ::nightfall_m68k_instruction_hook(unsigned int pc);

  bool valid_address(uint32_t address, size_t length) const;
  void write_word(uint32_t address, uint16_t value);
  void write_long(uint32_t address, uint32_t value);
  void on_instruction(uint32_t pc);
  bool handle_trap(uint16_t trap_word);
  bool is_implemented_trap(uint16_t trap_word) const;

  RuntimeOptions options_;
  std::vector<uint8_t> memory_;
  RuntimeResult result_{};
  bool loaded_ = false;
  bool stopped_ = false;
};

}  // namespace nightfall::m68k

#endif
