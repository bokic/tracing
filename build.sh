#!/usr/bin/env bash

set -e

rm -rf compile_commands.json trace.perfetto-trace

cmake -B build -DENABLE_PERFETTO=ON
cmake --build build

cp build/compile_commands.json .

./build/test

ls -la
