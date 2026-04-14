# Perfetto Automatic Function Tracing

This project demonstrates how to implement **automatic function-level tracing** in a C/C++ application using the [Perfetto SDK](https://perfetto.dev/) and the GCC/Clang `-finstrument-functions` compiler feature.

## Features

- **Automatic Instrumentation**: All function entry and exit points are automatically captured without requiring manual `TRACE_EVENT` macros in your business logic.
- **Zero-Boilerplate Lifecycle**: Tracing starts and stops automatically when the program loads and exits using compiler-level constructor/destructor hooks.
- **Symbol Demangling**: Human-readable function names are automatically resolved using `abi::__cxa_demangle`.
- **Modular Build System**: A standalone `tracing` module that can be easily integrated into any CMake project.
- **Robust Shutdown**: Ensures all trace data is flushed and synced to disk (`fsync`) even for short-lived executions.

## Project Structure

- `tracing/`: The core tracing module.
  - `tracing.cpp`: Implementation of `__cyg_profile_func_*` hooks and Perfetto session management.
  - `tracing.h`: Public interface for manual trace events.
  - `CMakeLists.txt`: Modular build configuration that fetches the Perfetto SDK.
- `test.cpp`: A sample application demonstrating automatic tracing.
- `build.sh`: A helper script to build and run the project.

## Usage

### 1. Build and Run
Use the provided `build.sh` script to compile and execute the test program:

```bash
./build.sh
```

This will:
1. Download the Perfetto SDK (amalgamated).
2. Build the `tracing` library and the `test` executable.
3. Run the program and generate a trace file named `{process_name}_{timestamp}.perfetto` (e.g., `test_2026.04.15_00.07.15.perfetto`).

### 2. View the Trace
Open [ui.perfetto.dev](https://ui.perfetto.dev/) in your browser and drag the generated `.perfetto` file into the UI.

## Configuration

You can configure the tracing behavior at runtime using environment variables:

- `PERFETTO_TRACE_FILE`: Specify the path where the trace file should be saved. 
  - Example: `PERFETTO_TRACE_FILE=my_app.perfetto ./my_app`
  - Default: `{process_name}_{timestamp}.perfetto`

## Integration Guide

To enable automatic tracing in your own target, add the following to your `CMakeLists.txt`:

```cmake
# Add the tracing subdirectory
add_subdirectory(tracing)

# Create your target
add_executable(my_app main.cpp)

# Enable automatic tracing for your target
target_trace_enable(my_app)
```

You can toggle tracing globally using the `ENABLE_PERFETTO` CMake option:

```bash
cmake -B build -DENABLE_PERFETTO=ON
```

## Technical Details

- **Instrumentation**: The project uses `-finstrument-functions` for automatic hooking. The `tracing.cpp` file is compiled **without** instrumentation to avoid infinite recursion.
- **Recursion Guard**: Uses a `thread_local` flag to prevent the tracing hooks from being traced themselves.
- **Lifecycle**:
    - `perfetto_load()`: Marked with `__attribute__((constructor))` to initialize the session before `main()`.
    - `perfetto_unload()`: Marked with `__attribute__((destructor))` to flush and stop the session after `main()`.
