#include "nightfall.h"

void trace_good(nf_trace_callback trace, void *user_data) {
  const nf_trace_event event{"core", "fixture.good", 1u};
  trace(&event, user_data);
}
