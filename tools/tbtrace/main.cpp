#include "M68KRuntime.h"
#include "ResourceFork.h"

#include <sys/types.h>
#include <sys/xattr.h>

#include <stdint.h>

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr const char *kResourceForkAttribute = "com.apple.ResourceFork";

std::string status_name(nf_status status) {
  switch (status) {
    case NF_OK:
      return "ok";
    case NF_ERROR_INVALID_ARGUMENT:
      return "invalid-argument";
    case NF_ERROR_UNIMPLEMENTED:
      return "unimplemented";
    case NF_ERROR_UNSUPPORTED_INSTRUCTION:
      return "unsupported-instruction";
    case NF_ERROR_INCOMPLETE_PROGRAM:
      return "incomplete-program";
    case NF_ERROR_RESOURCE_EXHAUSTED:
      return "resource-exhausted";
    default:
      return "unknown";
  }
}

bool read_resource_fork_xattr(const std::string &path, std::vector<uint8_t> *out_bytes) {
  const ssize_t size = getxattr(path.c_str(), kResourceForkAttribute, nullptr, 0u, 0u, 0);
  if (size <= 0) {
    return false;
  }

  out_bytes->assign(static_cast<size_t>(size), 0u);
  const ssize_t read_size = getxattr(path.c_str(), kResourceForkAttribute, out_bytes->data(), out_bytes->size(), 0u, 0);
  return read_size == size;
}

bool parse_u32(const char *text, uint32_t *out_value) {
  char *end = nullptr;
  const int base = std::strncmp(text, "0x", 2u) == 0 || std::strncmp(text, "0X", 2u) == 0 ? 16 : 10;
  const unsigned long parsed = std::strtoul(text, &end, base);
  if (end == text || *end != '\0' || parsed > 0xFFFFFFFFul) {
    return false;
  }
  *out_value = static_cast<uint32_t>(parsed);
  return true;
}

void print_trap_list(const char *label, const std::vector<uint16_t> &traps) {
  std::cout << label << ":";
  if (traps.empty()) {
    std::cout << " none\n";
    return;
  }

  for (uint16_t trap : traps) {
    std::cout << " 0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << trap << std::dec
              << std::nouppercase << std::setfill(' ');
  }
  std::cout << "\n";
}

}  // namespace

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "usage: tbtrace <module-path> [--cycles N] [--entry-offset N]\n";
    return 2;
  }

  uint32_t cycle_budget = 1000000u;
  uint32_t entry_offset = 0u;
  for (int index = 2; index < argc; ++index) {
    if (std::strcmp(argv[index], "--cycles") == 0 && index + 1 < argc) {
      if (!parse_u32(argv[index + 1], &cycle_budget)) {
        std::cerr << "invalid cycle budget\n";
        return 2;
      }
      ++index;
      continue;
    }
    if (std::strcmp(argv[index], "--entry-offset") == 0 && index + 1 < argc) {
      if (!parse_u32(argv[index + 1], &entry_offset)) {
        std::cerr << "invalid entry offset\n";
        return 2;
      }
      ++index;
      continue;
    }
    std::cerr << "unknown argument: " << argv[index] << "\n";
    return 2;
  }

  std::vector<uint8_t> resource_fork_bytes;
  if (!read_resource_fork_xattr(argv[1], &resource_fork_bytes)) {
    std::cerr << "could not read macOS resource fork xattr\n";
    return 1;
  }

  nightfall::resource::ResourceFork fork{};
  const nf_status parse_status =
      nightfall::resource::parse_resource_fork(resource_fork_bytes.data(), resource_fork_bytes.size(), &fork);
  if (parse_status != NF_OK) {
    std::cerr << "resource fork parse failed: " << status_name(parse_status) << "\n";
    return 1;
  }

  const uint32_t adgm = nightfall::resource::make_type_code('A', 'D', 'g', 'm');
  const nightfall::resource::ResourceEntry *program = nightfall::resource::find_resource(fork, adgm, 0);
  if (program == nullptr) {
    std::cerr << "resource ADgm/0 not found\n";
    return 1;
  }

  nightfall::m68k::RuntimeOptions options{};
  options.cycle_budget = cycle_budget;
  options.entry_address = options.code_address + entry_offset;
  nightfall::m68k::Runtime runtime(options);
  const nf_status load_status = runtime.load_program(program->data.data(), program->data.size());
  if (load_status != NF_OK) {
    std::cerr << "program load failed: " << status_name(load_status) << "\n";
    return 1;
  }

  const nightfall::m68k::RuntimeResult result = runtime.run();
  std::cout << "resource-fork-bytes: " << resource_fork_bytes.size() << "\n";
  std::cout << "resource-count: " << fork.entries.size() << "\n";
  std::cout << "ADgm-count: " << nightfall::resource::count_type(fork, adgm) << "\n";
  std::cout << "ADgm-0-bytes: " << program->data.size() << "\n";
  std::cout << "status: " << status_name(result.status) << "\n";
  std::cout << "stop-reason: " << result.stop_reason << "\n";
  std::cout << "cycles-run: " << result.cycles_run << "\n";
  std::cout << "final-pc: 0x" << std::hex << std::uppercase << result.final_pc << std::dec << std::nouppercase << "\n";
  std::cout << "trap-count: " << result.traps.size() << "\n";
  std::cout << "unimplemented-trap-count: " << result.unimplemented_traps.size() << "\n";
  print_trap_list("traps", result.traps);
  print_trap_list("unimplemented-traps", result.unimplemented_traps);

  return result.unimplemented_traps.empty() ? 0 : 1;
}
