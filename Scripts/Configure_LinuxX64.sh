#!/bin/bash
# Configures the build in ./build. Usage: Scripts/Configure_LinuxX64.sh [Debug|Release]
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cmake -G Ninja -DCMAKE_BUILD_TYPE="${1:-Release}" \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
      -S "$ROOT" -B "$ROOT/build"
