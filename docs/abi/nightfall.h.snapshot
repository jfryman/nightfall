#ifndef NIGHTFALL_CORE_NIGHTFALL_H
#define NIGHTFALL_CORE_NIGHTFALL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NF_ABI_VERSION 1u

typedef enum nf_status {
  NF_OK = 0,
  NF_ERROR_INVALID_ARGUMENT = 1,
  NF_ERROR_UNIMPLEMENTED = 2,
  NF_ERROR_UNSUPPORTED_INSTRUCTION = 3,
  NF_ERROR_INCOMPLETE_PROGRAM = 4,
  NF_ERROR_RESOURCE_EXHAUSTED = 5,
} nf_status;

typedef struct nf_version {
  uint32_t abi;
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
} nf_version;

typedef struct nf_trace_event {
  const char *category;
  const char *name;
  uint64_t value;
} nf_trace_event;

typedef void (*nf_trace_callback)(const nf_trace_event *event, void *user_data);

typedef struct nf_context_options {
  nf_trace_callback trace;
  void *trace_user_data;
} nf_context_options;

typedef struct nf_context nf_context;

typedef nf_status (*nf_trap_handler)(nf_context *context, uint16_t trap_word, void *user_data);

typedef struct nf_execution_result {
  size_t bytes_consumed;
  uint16_t last_word;
  nf_status status;
} nf_execution_result;

nf_version nf_get_version(void);
nf_status nf_context_create(const nf_context_options *options, nf_context **out_context);
void nf_context_destroy(nf_context *context);
nf_status nf_context_tick(nf_context *context, uint32_t ticks);
uint64_t nf_context_ticks(const nf_context *context);
nf_status nf_context_set_random_seed(nf_context *context, uint32_t seed);
int nf_trap_is_aline(uint16_t trap_word);
nf_status nf_context_register_trap(nf_context *context,
                                   uint16_t trap_word,
                                   nf_trap_handler handler,
                                   void *user_data);
nf_status nf_context_dispatch_trap(nf_context *context, uint16_t trap_word);
nf_status nf_context_execute_fixture(nf_context *context,
                                     const uint8_t *bytes,
                                     size_t byte_count,
                                     nf_execution_result *out_result);
nf_status nf_module_load(nf_context *context, const char *path);
nf_status nf_module_start(nf_context *context, uint32_t width, uint32_t height);
nf_status nf_advance(nf_context *context, uint32_t virtual_ticks);
nf_status nf_module_framebuffer(const nf_context *context,
                                const uint8_t **out_pixels,
                                uint32_t *out_width,
                                uint32_t *out_height,
                                uint32_t *out_stride);
nf_status nf_module_stop(nf_context *context);

#ifdef __cplusplus
}
#endif

#endif
