#pragma once

#include "common/CacheTypes.h"

#include <string>
#include <vector>

namespace b5cache {

struct ComparisonPlan {
    std::string name;
    SimulationConfig config;
};

struct ComparisonResult {
    std::string planName;
    StatisticsSnapshot statistics;
};

class ComparisonRunner final {
public:
    static std::vector<ComparisonResult> Run(
        const std::vector<ComparisonPlan>& plans,
        const std::vector<MemoryAccess>& trace);
};

}  // namespace b5cache
