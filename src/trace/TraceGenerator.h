#pragma once

#include "common/CacheTypes.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace b5cache {

enum class TraceGenerationMode {
    Sequential,
    Loop,
    Random,
    HotSet,
    MixedReadWrite
};

struct TraceGenerationConfig {
    TraceGenerationMode mode = TraceGenerationMode::Sequential;
    std::size_t requestCount = 32;
    std::uint64_t startAddress = 0;
    std::uint64_t addressRangeBytes = 256;
    std::uint64_t stepBytes = 16;
    std::size_t loopLength = 4;
    std::size_t hotSetSize = 2;
    double hotProbability = 0.8;
    double writeProbability = 0.0;
    std::uint64_t randomSeed = 20260903;
};

class TraceGenerator final {
public:
    static std::vector<MemoryAccess> Generate(const TraceGenerationConfig& config);
    static std::string FormatText(const std::vector<MemoryAccess>& accesses);
};

}  // namespace b5cache
