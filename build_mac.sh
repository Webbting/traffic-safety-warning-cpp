#!/bin/zsh
set -e

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

mkdir -p build-mac

if command -v cmake >/dev/null 2>&1; then
  cmake -S . -B build-mac
  cmake --build build-mac
else
  c++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o build-mac/traffic_safety_ai
fi

echo
echo "Build complete:"
echo "  $ROOT/build-mac/traffic_safety_ai"
echo
echo "Try:"
echo "  ./build-mac/traffic_safety_ai --csv data/sample_detections.csv"
echo "  ./build-mac/traffic_safety_ai --csv data/sample_detections.csv --meters-per-pixel 0.05 --fps 1"
