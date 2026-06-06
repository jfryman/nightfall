#include "nightfall.h"

#include "M68KRuntime.h"
#include "ResourceFork.h"

#include <sys/types.h>
#include <sys/xattr.h>

#include <algorithm>
#include <limits>
#include <vector>

namespace {

constexpr uint16_t kALineTrapMask = 0xF000u;
constexpr uint16_t kALineTrapPrefix = 0xA000u;
constexpr uint16_t kRtsOpcode = 0x4E75u;
constexpr size_t kALineTrapCount = 0x1000u;
constexpr size_t kContextPoolSize = 4u;
constexpr const char *kResourceForkAttribute = "com.apple.ResourceFork";

struct TrapEntry {
  nf_trap_handler handler;
  void *user_data;
};

size_t trap_index(uint16_t trap_word) {
  return static_cast<size_t>(trap_word & 0x0FFFu);
}

uint16_t read_big_endian_word(const uint8_t *bytes) {
  return static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8u) | bytes[1]);
}

bool read_resource_fork_xattr(const char *path, std::vector<uint8_t> *out_bytes) {
  const ssize_t size = getxattr(path, kResourceForkAttribute, nullptr, 0u, 0u, 0);
  if (size <= 0) {
    return false;
  }

  out_bytes->assign(static_cast<size_t>(size), 0u);
  const ssize_t read_size = getxattr(path, kResourceForkAttribute, out_bytes->data(), out_bytes->size(), 0u, 0);
  return read_size == size;
}

bool framebuffer_size_fits(uint32_t width, uint32_t height) {
  if (width == 0u || height == 0u) {
    return false;
  }

  const size_t max = std::numeric_limits<size_t>::max();
  return static_cast<size_t>(width) <= max / static_cast<size_t>(height) / 4u;
}

}  // namespace

struct nf_context {
  bool in_use;
  nf_trace_callback trace;
  void *trace_user_data;
  uint64_t ticks;
  uint32_t random_seed;
  TrapEntry traps[kALineTrapCount];
  bool module_loaded;
  bool module_running;
  std::vector<uint8_t> module_program;
  std::vector<uint8_t> framebuffer;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint32_t framebuffer_stride;
};

namespace {

nf_context g_context_pool[kContextPoolSize]{};

void emit_trace(nf_context *context, const char *category, const char *name, uint64_t value) {
  if (context == nullptr || context->trace == nullptr) {
    return;
  }

  const nf_trace_event event{
      category,
      name,
      value,
  };
  context->trace(&event, context->trace_user_data);
}

bool is_pool_context(const nf_context *context) {
  for (size_t index = 0u; index < kContextPoolSize; ++index) {
    if (context == &g_context_pool[index]) {
      return true;
    }
  }
  return false;
}

}  // namespace

nf_version nf_get_version(void) {
  return nf_version{
      NF_ABI_VERSION,
      0u,
      1u,
      0u,
  };
}

nf_status nf_context_create(const nf_context_options *options, nf_context **out_context) {
  if (out_context == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  nf_context *context = nullptr;
  for (size_t index = 0u; index < kContextPoolSize; ++index) {
    if (!g_context_pool[index].in_use) {
      context = &g_context_pool[index];
      break;
    }
  }

  if (context == nullptr) {
    *out_context = nullptr;
    return NF_ERROR_RESOURCE_EXHAUSTED;
  }

  *context = nf_context{};
  context->in_use = true;
  context->trace = options != nullptr ? options->trace : nullptr;
  context->trace_user_data = options != nullptr ? options->trace_user_data : nullptr;

  *out_context = context;
  emit_trace(context, "core", "context.create", 0u);
  return NF_OK;
}

void nf_context_destroy(nf_context *context) {
  if (context == nullptr) {
    return;
  }

  emit_trace(context, "core", "context.destroy", context->ticks);
  if (is_pool_context(context)) {
    *context = nf_context{};
  }
}

nf_status nf_context_tick(nf_context *context, uint32_t ticks) {
  if (context == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  context->ticks += ticks;
  emit_trace(context, "core", "context.tick", context->ticks);
  return NF_OK;
}

uint64_t nf_context_ticks(const nf_context *context) {
  if (context == nullptr) {
    return 0u;
  }

  return context->ticks;
}

nf_status nf_context_set_random_seed(nf_context *context, uint32_t seed) {
  if (context == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  context->random_seed = seed;
  emit_trace(context, "core", "context.random_seed", seed);
  return NF_OK;
}

// Source: Apple A/UX Toolbox Macintosh ROM Interface, Appendix C,
// "A-line traps", pp. C-3-C-5. This implements only dispatch scaffolding,
// not Toolbox routine behavior.
int nf_trap_is_aline(uint16_t trap_word) {
  return (trap_word & kALineTrapMask) == kALineTrapPrefix ? 1 : 0;
}

nf_status nf_context_register_trap(nf_context *context,
                                   uint16_t trap_word,
                                   nf_trap_handler handler,
                                   void *user_data) {
  if (context == nullptr || handler == nullptr || nf_trap_is_aline(trap_word) == 0) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  context->traps[trap_index(trap_word)] = TrapEntry{
      handler,
      user_data,
  };
  emit_trace(context, "core", "trap.register", trap_word);
  return NF_OK;
}

nf_status nf_context_dispatch_trap(nf_context *context, uint16_t trap_word) {
  if (context == nullptr || nf_trap_is_aline(trap_word) == 0) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  TrapEntry entry = context->traps[trap_index(trap_word)];
  if (entry.handler == nullptr) {
    emit_trace(context, "core", "trap.unimplemented", trap_word);
    return NF_ERROR_UNIMPLEMENTED;
  }

  emit_trace(context, "core", "trap.dispatch", trap_word);
  return entry.handler(context, trap_word, entry.user_data);
}

nf_status nf_context_execute_fixture(nf_context *context,
                                     const uint8_t *bytes,
                                     size_t byte_count,
                                     nf_execution_result *out_result) {
  if (context == nullptr || bytes == nullptr || out_result == nullptr || (byte_count % 2u) != 0u) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_result = nf_execution_result{
      0u,
      0u,
      NF_ERROR_INCOMPLETE_PROGRAM,
  };

  for (size_t offset = 0u; offset < byte_count; offset += 2u) {
    const uint16_t word = read_big_endian_word(bytes + offset);
    out_result->bytes_consumed = offset + 2u;
    out_result->last_word = word;

    if (nf_trap_is_aline(word) != 0) {
      const nf_status status = nf_context_dispatch_trap(context, word);
      out_result->status = status;
      if (status != NF_OK) {
        return status;
      }
      continue;
    }

    if (word == kRtsOpcode) {
      emit_trace(context, "core", "fixture.return", word);
      out_result->status = NF_OK;
      return NF_OK;
    }

    emit_trace(context, "core", "fixture.unsupported", word);
    out_result->status = NF_ERROR_UNSUPPORTED_INSTRUCTION;
    return NF_ERROR_UNSUPPORTED_INSTRUCTION;
  }

  emit_trace(context, "core", "fixture.incomplete", out_result->last_word);
  return NF_ERROR_INCOMPLETE_PROGRAM;
}

// Source: docs/clean-room-sources.md, "Phase 5 Resource Fork Runner Backfill".
nf_status nf_module_load(nf_context *context, const char *path) {
  if (context == nullptr || path == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  std::vector<uint8_t> resource_fork_bytes;
  if (!read_resource_fork_xattr(path, &resource_fork_bytes)) {
    emit_trace(context, "module", "load.resource_fork_missing", 0u);
    return NF_ERROR_INVALID_ARGUMENT;
  }

  nightfall::resource::ResourceFork fork{};
  const nf_status parse_status =
      nightfall::resource::parse_resource_fork(resource_fork_bytes.data(), resource_fork_bytes.size(), &fork);
  if (parse_status != NF_OK) {
    emit_trace(context, "module", "load.resource_fork_invalid", parse_status);
    return parse_status;
  }

  const uint32_t adgm = nightfall::resource::make_type_code('A', 'D', 'g', 'm');
  const nightfall::resource::ResourceEntry *program = nightfall::resource::find_resource(fork, adgm, 0);
  if (program == nullptr || program->data.empty()) {
    emit_trace(context, "module", "load.adgm0_missing", nightfall::resource::count_type(fork, adgm));
    return NF_ERROR_INVALID_ARGUMENT;
  }

  context->module_program = program->data;
  context->module_loaded = true;
  context->module_running = false;
  context->framebuffer.clear();
  context->framebuffer_width = 0u;
  context->framebuffer_height = 0u;
  context->framebuffer_stride = 0u;
  emit_trace(context, "module", "load.adgm0", context->module_program.size());
  return NF_OK;
}

// Source: docs/clean-room-sources.md, "Phase 5 Resource Fork Runner Backfill".
nf_status nf_module_start(nf_context *context, uint32_t width, uint32_t height) {
  if (context == nullptr || !context->module_loaded || !framebuffer_size_fits(width, height)) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  context->framebuffer_width = width;
  context->framebuffer_height = height;
  context->framebuffer_stride = width * 4u;
  context->framebuffer.assign(static_cast<size_t>(context->framebuffer_stride) * height, 0u);

  nightfall::m68k::Runtime runtime;
  const nf_status load_status = runtime.load_program(context->module_program.data(), context->module_program.size());
  if (load_status != NF_OK) {
    emit_trace(context, "module", "start.program_invalid", load_status);
    return load_status;
  }

  const nightfall::m68k::RuntimeResult result = runtime.run();
  emit_trace(context, "module", "start.traps", result.traps.size());
  emit_trace(context, "module", "start.unimplemented_traps", result.unimplemented_traps.size());
  if (result.status != NF_OK) {
    context->module_running = false;
    return result.status;
  }

  context->module_running = true;
  emit_trace(context, "module", "start", context->ticks);
  return NF_OK;
}

nf_status nf_advance(nf_context *context, uint32_t virtual_ticks) {
  if (context == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  context->ticks += virtual_ticks;
  emit_trace(context, "core", "advance", context->ticks);
  return NF_OK;
}

nf_status nf_module_framebuffer(const nf_context *context,
                                const uint8_t **out_pixels,
                                uint32_t *out_width,
                                uint32_t *out_height,
                                uint32_t *out_stride) {
  if (context == nullptr || out_pixels == nullptr || out_width == nullptr || out_height == nullptr ||
      out_stride == nullptr || !context->module_running || context->framebuffer.empty()) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  *out_pixels = context->framebuffer.data();
  *out_width = context->framebuffer_width;
  *out_height = context->framebuffer_height;
  *out_stride = context->framebuffer_stride;
  return NF_OK;
}

nf_status nf_module_stop(nf_context *context) {
  if (context == nullptr) {
    return NF_ERROR_INVALID_ARGUMENT;
  }

  context->module_running = false;
  emit_trace(context, "module", "stop", context->ticks);
  return NF_OK;
}
