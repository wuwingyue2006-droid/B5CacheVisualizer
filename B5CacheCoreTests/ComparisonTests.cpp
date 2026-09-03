#include "TestSuites.h"

#include "experiment/ComparisonRunner.h"
#include "core/CacheSimulator.h"

#include <stdexcept>

namespace b5cache::tests {
namespace {

SimulationConfig DirectConfig() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1 = {"L1", 64, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    config.l2 = {"L2", 128, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    return config;
}

SimulationConfig SetLruConfig() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1 = {"L1", 64, 16, 2, MappingKind::SetAssociative, ReplacementKind::Lru};
    config.l2 = {"L2", 128, 16, 2, MappingKind::SetAssociative, ReplacementKind::Lru};
    return config;
}

void TestRunsIndependentPlansAgainstTheSameTrace() {
    const std::vector<MemoryAccess> trace = {
        {0x00, false}, {0x40, false}, {0x00, false}, {0x80, true}, {0x00, false},
    };
    const std::vector<ComparisonPlan> plans = {
        {"Direct + FIFO", DirectConfig()},
        {"2-way Set + LRU", SetLruConfig()},
    };

    const auto first = ComparisonRunner::Run(plans, trace);
    const auto second = ComparisonRunner::Run(plans, trace);
    Require(first.size() == 2 && second.size() == 2, "Runner should return one result per plan.");
    Require(first[0].statistics.accesses == trace.size() && first[1].statistics.accesses == trace.size(),
            "Every plan must receive the whole, identical trace.");
    Require(first[0].statistics.accesses == second[0].statistics.accesses &&
                first[0].statistics.memoryMisses == second[0].statistics.memoryMisses &&
                first[1].statistics.memoryMisses == second[1].statistics.memoryMisses,
            "Every run must start with independent simulator state.");
}

void TestRejectsInvalidPlanCollections() {
    const std::vector<MemoryAccess> trace = {{0x00, false}};
    try {
        static_cast<void>(ComparisonRunner::Run({{"Only one", DirectConfig()}}, trace));
    } catch (const std::invalid_argument&) {
        // Expected.
    }

    try {
        static_cast<void>(ComparisonRunner::Run(
            {{"Duplicate", DirectConfig()}, {"Duplicate", SetLruConfig()}}, trace));
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error("Invalid comparison plans should be rejected.");
}

}  // namespace

void AddComparisonTests(TestList& tests) {
    tests.push_back({"Comparison: independent plans use the same trace", TestRunsIndependentPlansAgainstTheSameTrace});
    tests.push_back({"Comparison: plan validation", TestRejectsInvalidPlanCollections});
}

}  // namespace b5cache::tests
