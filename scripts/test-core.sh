#!/usr/bin/env bash
# Linux equivalent of scripts/test.ps1 for the MFC-free core test project.
# The MFC dialog (B5CacheVisualizer) still requires Windows and Visual Studio 2022.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
out_dir="${root}/bin/linux/Debug"
mkdir -p "${out_dir}"

echo "Building B5CacheCoreTests with g++ (C++17, Debug)..."
g++ -std=c++17 -Wall -Wextra -D_CONSOLE -D_DEBUG \
    -I "${root}/src" \
    -I "${root}/B5CacheCoreTests" \
    "${root}/B5CacheCoreTests/CoreTests.cpp" \
    "${root}/B5CacheCoreTests/MappingTests.cpp" \
    "${root}/B5CacheCoreTests/ReplacementTests.cpp" \
    "${root}/B5CacheCoreTests/StatisticsTests.cpp" \
    "${root}/B5CacheCoreTests/TestMain.cpp" \
    "${root}/B5CacheCoreTests/TraceTests.cpp" \
    "${root}/src/common/CacheTypes.cpp" \
    "${root}/src/core/CacheLevel.cpp" \
    "${root}/src/core/CacheSimulator.cpp" \
    "${root}/src/mapping/MappingStrategies.cpp" \
    "${root}/src/replacement/ReplacementPolicies.cpp" \
    "${root}/src/statistics/CacheStatistics.cpp" \
    "${root}/src/trace/MemoryTraceParser.cpp" \
    -o "${out_dir}/B5CacheCoreTests"

echo "Running core tests..."
"${out_dir}/B5CacheCoreTests"
