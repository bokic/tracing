#pragma once

#ifdef ENABLE_PERFETTO
 #ifdef __cplusplus
 extern "C" {
 #endif
    // Automatic tracing helpers
    __attribute__((visibility("default"))) void trace_delay_start(const char* name);
    __attribute__((visibility("default"))) void trace_delay_end(void);
    __attribute__((visibility("default"))) void trace_net_start(const char* name);
    __attribute__((visibility("default"))) void trace_net_end(void);
 #ifdef __cplusplus
 }
 #endif
#else
#define trace_delay_start(name) 
#define trace_delay_end() 
#endif
