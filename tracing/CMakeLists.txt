set(ENABLE_PERFETTO OFF CACHE BOOL "Enable perfetto tracing")

# Define the function to enable tracing for a specific target
function(target_trace_enable target)
    if (ENABLE_PERFETTO)
        target_link_libraries(${target} PRIVATE tracing "-rdynamic")
        target_compile_definitions(${target} PRIVATE ENABLE_PERFETTO)
        
        # Enable instrument-functions for the target source files
        target_compile_options(${target} PRIVATE "-finstrument-functions")
    endif()
endfunction()

if (NOT ENABLE_PERFETTO)
    return()
endif()

include(FetchContent)
FetchContent_Declare(
    perfetto
    GIT_REPOSITORY https://github.com/google/perfetto.git
    GIT_TAG v44.0
    GIT_SHALLOW TRUE
)

# download perfetto
FetchContent_MakeAvailable(perfetto)

# Use the amalgamated SDK files from the cloned repository
add_library(perfetto STATIC ${perfetto_SOURCE_DIR}/sdk/perfetto.cc)
target_include_directories(perfetto PUBLIC ${perfetto_SOURCE_DIR}/sdk)

find_package(Threads REQUIRED)
target_link_libraries(perfetto PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

# Set default symbol visibility to hidden, and enable -fPIC so the static
# archive can be linked into libcfrds.so without relocation errors.
# -g0: no debug info (overrides global CMAKE_*_FLAGS_DEBUG) — keeps .so lean.
# -O2: needed for LTO to do meaningful dead-code elimination (overrides -O0).
# INTERPROCEDURAL_OPTIMIZATION: emit LTO bitcode so unused perfetto code is
# stripped when libcfrds.so is linked.
set_target_properties(perfetto PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    INTERPROCEDURAL_OPTIMIZATION ON
)
target_compile_options(perfetto PRIVATE "-g0" "-O2")

# Internal library for tracing hooks (now mixed C and C++)
add_library(tracing STATIC 
    tracing.c
    perfetto_backend.cpp
)
target_link_libraries(tracing PUBLIC perfetto)
target_compile_definitions(tracing PUBLIC ENABLE_PERFETTO)

# Set default symbol visibility to hidden, and enable -fPIC so the static
# archive can be linked into libcfrds.so without relocation errors.
# -g0 / -O2: same rationale as perfetto above.
set_target_properties(tracing PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    INTERPROCEDURAL_OPTIMIZATION ON
)
target_compile_options(tracing PRIVATE "-g0" "-O2")

# _Thread_local defaults to local-exec TLS model which is incompatible with
# shared libraries. Force global-dynamic so libtracing.a can be linked into .so.
# Also need -fPIC since the static archive gets linked into a shared library.
target_compile_options(tracing PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-fPIC -ftls-model=global-dynamic>
)

# Allow targets linking to 'tracing' to find the header
target_include_directories(tracing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
