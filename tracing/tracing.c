#define _GNU_SOURCE
#include "tracing_backend.h"
#include <stdatomic.h>
#include <threads.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

static _Thread_local bool in_hook = false;
static atomic_bool perfetto_ready = false;

// Buffer for dynamic filename
static char dynamic_trace_path[512];

// Public tracing interface implemented in C
void trace_delay_start(const char* name) __attribute__((no_instrument_function));
void trace_delay_end(void) __attribute__((no_instrument_function));

void trace_delay_start(const char* name) {
    if (!atomic_load(&perfetto_ready)) return;
    perf_backend_trace_delay_start(name);
}

void trace_delay_end(void) {
    if (!atomic_load(&perfetto_ready)) return;
    perf_backend_trace_delay_end();
}

// Compiler instrumentation hooks
__attribute__((visibility("default"), no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    if (!atomic_load(&perfetto_ready) || in_hook) return;
    in_hook = true;
    perf_backend_on_func_enter(this_fn, call_site);
    in_hook = false;
}

__attribute__((visibility("default"), no_instrument_function))
void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    if (!atomic_load(&perfetto_ready) || in_hook) return;
    in_hook = true;
    perf_backend_on_func_exit(this_fn, call_site);
    in_hook = false;
}

// Lifecycle management
void perfetto_load(void) __attribute__((constructor, no_instrument_function));
void perfetto_unload(void) __attribute__((destructor, no_instrument_function));

void perfetto_load(void) {
    const char* trace_path;
    const char* env_path = getenv("PERFETTO_TRACE_FILE");
    
    if (env_path) {
        trace_path = env_path;
    } else {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y.%m.%d_%H.%M.%S", t);

        const char* proc_name = program_invocation_short_name;
        if (!proc_name) proc_name = "unknown";

        snprintf(dynamic_trace_path, sizeof(dynamic_trace_path), "%s_%s.perfetto", proc_name, timestamp);
        trace_path = dynamic_trace_path;
    }

    perf_backend_init();
    perf_backend_start(trace_path);
    atomic_store(&perfetto_ready, true);
}

void perfetto_unload(void) {
    atomic_store(&perfetto_ready, false);
    perf_backend_stop();
}
