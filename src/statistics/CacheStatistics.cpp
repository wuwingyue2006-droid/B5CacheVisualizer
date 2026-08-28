#include "statistics/CacheStatistics.h"

namespace b5cache {

void CacheStatistics::Reset() noexcept {
    snapshot_ = {};
}

void CacheStatistics::Record(const AccessResult& result) noexcept {
    ++snapshot_.accesses;
    result.request.isWrite ? ++snapshot_.writes : ++snapshot_.reads;

    switch (result.outcome) {
    case AccessOutcome::L1Hit:
        ++snapshot_.l1Hits;
        break;
    case AccessOutcome::L2Hit:
        ++snapshot_.l2Hits;
        break;
    case AccessOutcome::MemoryMiss:
        ++snapshot_.memoryMisses;
        break;
    }
}

StatisticsSnapshot CacheStatistics::Snapshot() const noexcept {
    return snapshot_;
}

}  // namespace b5cache
