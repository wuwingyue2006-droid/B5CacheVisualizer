#pragma once

#include "common/CacheTypes.h"

namespace b5cache {

class CacheStatistics {
public:
    void Reset() noexcept;
    void Record(const AccessResult& result) noexcept;
    StatisticsSnapshot Snapshot() const noexcept;

private:
    StatisticsSnapshot snapshot_;
};

}  // namespace b5cache
