#pragma once

#include "common/CacheTypes.h"
#include "core/CacheLevel.h"
#include "statistics/CacheStatistics.h"

#include <vector>

namespace b5cache {

class CacheSimulator {
public:
    static SimulationConfig DefaultConfig();

    explicit CacheSimulator(SimulationConfig config = DefaultConfig());

    AccessResult Access(const MemoryAccess& request);
    std::vector<AccessResult> Run(const std::vector<MemoryAccess>& trace);
    void Reset();

    const SimulationConfig& Config() const noexcept;
    const CacheLevel& L1() const noexcept;
    const CacheLevel& L2() const noexcept;
    StatisticsSnapshot Statistics() const noexcept;

private:
    SimulationConfig config_;
    CacheLevel l1_;
    CacheLevel l2_;
    CacheStatistics statistics_;
    std::uint64_t tick_ = 0;
};

}  // namespace b5cache
