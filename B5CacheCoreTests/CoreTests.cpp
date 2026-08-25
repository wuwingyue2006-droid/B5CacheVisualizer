#include "TestSuites.h"

#include "core/CacheSimulator.h"

#include <stdexcept>

namespace b5cache::tests {
namespace {

void TestDirectMappingHierarchy() {
    CacheSimulator simulator;

    const auto first = simulator.Access({0x00, false});
    const auto repeated = simulator.Access({0x00, false});
    const auto conflict = simulator.Access({0x40, true});
    const auto restored = simulator.Access({0x00, false});

    Require(first.outcome == AccessOutcome::MemoryMiss, "First access should miss memory.");
    Require(repeated.outcome == AccessOutcome::L1Hit, "Repeated access should hit L1.");
    Require(conflict.outcome == AccessOutcome::MemoryMiss, "New block should miss memory.");
    Require(conflict.l1.evicted && conflict.l1.evictedBlock == 0, "Direct mapping should evict L1 block 0.");
    Require(restored.outcome == AccessOutcome::L2Hit, "Evicted L1 block should still hit L2.");
}

void TestReset() {
    CacheSimulator simulator;
    simulator.Access({0x00, false});
    simulator.Reset();

    Require(simulator.Statistics().accesses == 0, "Reset should clear statistics.");
    Require(simulator.Access({0x00, false}).outcome == AccessOutcome::MemoryMiss,
            "Reset should clear cache lines.");
}

void TestInvalidConfiguration() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1.sizeBytes = 65;

    bool rejected = false;
    try {
        CacheSimulator simulator(config);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    Require(rejected, "Invalid cache size should be rejected.");
}

}  // namespace

void AddCoreTests(TestList& tests) {
    tests.push_back({"Core: direct mapping hierarchy", TestDirectMappingHierarchy});
    tests.push_back({"Core: reset", TestReset});
    tests.push_back({"Core: invalid configuration", TestInvalidConfiguration});
}

}  // namespace b5cache::tests
