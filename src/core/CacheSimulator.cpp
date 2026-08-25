#include "core/CacheSimulator.h"

#include <utility>

namespace b5cache {

SimulationConfig CacheSimulator::DefaultConfig() {
    return {
        {"L1", 64, 16, 1, MappingKind::Direct, ReplacementKind::Fifo},
        {"L2", 128, 16, 1, MappingKind::Direct, ReplacementKind::Fifo},
    };
}

CacheSimulator::CacheSimulator(SimulationConfig config)
    : config_(std::move(config)), l1_(config_.l1), l2_(config_.l2) {}

AccessResult CacheSimulator::Access(const MemoryAccess& request) {
    ++tick_;
    AccessResult result;
    result.request = request;

    const auto l1Block = request.address / config_.l1.blockSizeBytes;
    result.l1 = l1_.Probe(l1Block, request.isWrite, tick_);
    if (result.l1.hit) {
        result.outcome = AccessOutcome::L1Hit;
        statistics_.Record(result);
        return result;
    }

    const auto l2Block = request.address / config_.l2.blockSizeBytes;
    result.l2 = l2_.Probe(l2Block, request.isWrite, tick_);
    if (result.l2.hit) {
        result.outcome = AccessOutcome::L2Hit;
    } else {
        result.outcome = AccessOutcome::MemoryMiss;
        const auto insertion = l2_.Insert(l2Block, request.isWrite, tick_);
        result.l2.lineIndex = insertion.lineIndex;
        result.l2.evicted = insertion.evicted;
        result.l2.evictedBlock = insertion.evictedBlock;
    }

    const auto l1Insertion = l1_.Insert(l1Block, request.isWrite, tick_);
    result.l1.lineIndex = l1Insertion.lineIndex;
    result.l1.evicted = l1Insertion.evicted;
    result.l1.evictedBlock = l1Insertion.evictedBlock;

    // TODO(A): harden this simplified non-inclusive, write-allocate flow for the full config matrix.
    statistics_.Record(result);
    return result;
}

std::vector<AccessResult> CacheSimulator::Run(const std::vector<MemoryAccess>& trace) {
    std::vector<AccessResult> results;
    results.reserve(trace.size());
    for (const auto& request : trace) {
        results.push_back(Access(request));
    }
    return results;
}

void CacheSimulator::Reset() {
    tick_ = 0;
    l1_.Reset();
    l2_.Reset();
    statistics_.Reset();
}

const SimulationConfig& CacheSimulator::Config() const noexcept {
    return config_;
}

const CacheLevel& CacheSimulator::L1() const noexcept {
    return l1_;
}

const CacheLevel& CacheSimulator::L2() const noexcept {
    return l2_;
}

StatisticsSnapshot CacheSimulator::Statistics() const noexcept {
    return statistics_.Snapshot();
}

}  // namespace b5cache
