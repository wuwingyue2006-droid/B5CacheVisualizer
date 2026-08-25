#include "common/CacheTypes.h"

namespace b5cache {

double StatisticsSnapshot::L1HitRate() const noexcept {
    return accesses == 0 ? 0.0 : static_cast<double>(l1Hits) / static_cast<double>(accesses);
}

double StatisticsSnapshot::L2HitRate() const noexcept {
    return accesses == 0 ? 0.0 : static_cast<double>(l2Hits) / static_cast<double>(accesses);
}

double StatisticsSnapshot::OverallHitRate() const noexcept {
    return accesses == 0
        ? 0.0
        : static_cast<double>(l1Hits + l2Hits) / static_cast<double>(accesses);
}

double StatisticsSnapshot::MissRate() const noexcept {
    return accesses == 0
        ? 0.0
        : static_cast<double>(memoryMisses) / static_cast<double>(accesses);
}

const char* ToString(const MappingKind value) noexcept {
    switch (value) {
    case MappingKind::Direct:
        return "Direct";
    case MappingKind::FullyAssociative:
        return "FullyAssociative";
    case MappingKind::SetAssociative:
        return "SetAssociative";
    }
    return "UnknownMapping";
}

const char* ToString(const ReplacementKind value) noexcept {
    switch (value) {
    case ReplacementKind::Fifo:
        return "FIFO";
    case ReplacementKind::Lru:
        return "LRU";
    }
    return "UnknownReplacement";
}

const char* ToString(const AccessOutcome value) noexcept {
    switch (value) {
    case AccessOutcome::L1Hit:
        return "L1 Hit";
    case AccessOutcome::L2Hit:
        return "L2 Hit";
    case AccessOutcome::MemoryMiss:
        return "Memory Miss";
    }
    return "UnknownOutcome";
}

}  // namespace b5cache
