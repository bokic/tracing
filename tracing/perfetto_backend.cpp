#include "perfetto.h"
#include "tracing_backend.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cxxabi.h>
#include <atomic>
#include <memory>

// Define categories used in the project
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("function_call").SetDescription("Automatic function tracing"),
    perfetto::Category("delay").SetDescription("Manual delay events")
);
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

static std::unique_ptr<perfetto::TracingSession> session;
static int trace_fd = -1;

extern "C" {

void perf_backend_init() {
    perfetto::TracingInitArgs args;
    args.backends = perfetto::kInProcessBackend;
    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();
}

void perf_backend_start(const char* filename) {
    perfetto::TraceConfig cfg;
    cfg.add_buffers()->set_size_kb(1024);
    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");

    cfg.set_write_into_file(true);
    cfg.set_file_write_period_ms(250);
    cfg.set_flush_period_ms(250);

    session = perfetto::Tracing::NewTrace();
    trace_fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (trace_fd < 0) {
        std::cerr << "Failed to open trace file: " << filename << std::endl;
        return;
    }
    session->Setup(cfg, trace_fd);
    session->StartBlocking();
}

void perf_backend_stop() {
    if (session) {
        session->FlushBlocking();
        session->StopBlocking();
        session.reset();
        usleep(20000); 
    }
    if (trace_fd >= 0) {
        fsync(trace_fd);
        close(trace_fd);
        trace_fd = -1;
    }
}

void perf_backend_on_func_enter(void* this_fn, void* call_site) {
    Dl_info info;
    if (dladdr(this_fn, &info) && info.dli_sname) {
        int status;
        char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        if (status == 0 && demangled) {
            TRACE_EVENT_BEGIN("function_call", perfetto::DynamicString{demangled});
            free(demangled);
        } else {
            TRACE_EVENT_BEGIN("function_call", perfetto::DynamicString{info.dli_sname});
        }
    } else {
        TRACE_EVENT_BEGIN("function_call", "unknown_function");
    }
}

void perf_backend_on_func_exit(void* this_fn, void* call_site) {
    TRACE_EVENT_END("function_call");
}

void perf_backend_trace_delay_start(const char* name) {
    TRACE_EVENT_BEGIN("delay", perfetto::DynamicString{name});
}

void perf_backend_trace_delay_end() {
    TRACE_EVENT_END("delay");
}

}
