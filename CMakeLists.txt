cmake_minimum_required(VERSION 3.10)

project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Add the tracing subdirectory (this provides target_trace_enable)
add_subdirectory(tracing)

add_executable(test
    test.cpp
)

# Use the custom function to enable tracing for the main executable
target_trace_enable(test)
