#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Internal C API for tracing hooks to communicate with the Perfetto backend
void perf_backend_init(void);
void perf_backend_start(const char* filename);
void perf_backend_stop(void);

void perf_backend_on_func_enter(void* this_fn, void* call_site);
void perf_backend_on_func_exit(void* this_fn, void* call_site);

void perf_backend_trace_delay_start(const char* name);
void perf_backend_trace_delay_end(void);

#ifdef __cplusplus
}
#endif
