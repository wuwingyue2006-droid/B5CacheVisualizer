#pragma once

#include "common/CacheTypes.h"

#include <cstddef>
#include <vector>

namespace b5cacheui {

using CacheSetsSnapshot = std::vector<std::vector<b5cache::CacheLineState>>;

struct VisualizationFrame {
    std::size_t frameNumber = 0;
    b5cache::AccessResult result;
    b5cache::StatisticsSnapshot statistics;
    b5cache::CacheLevelConfig l1Config;
    b5cache::CacheLevelConfig l2Config;
    CacheSetsSnapshot l1Sets;
    CacheSetsSnapshot l2Sets;
};

}  // namespace b5cacheui
