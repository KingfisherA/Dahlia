#!/bin/bash
# Configures the build in ./build. Usage: Scripts/Configure_Darwin_ARM64.sh [-c Debug|Release]
CONFIG=Debug
while getopts ":c:" opt; do
  case $opt in
    c) CONFIG="$OPTARG" ;;
    *) echo "Usage: $0 [-c Debug|Release]" >&2; exit 1 ;;
  esac
done

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cmake -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG" -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
      -S "$ROOT" -B "$ROOT/build"
