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

SimulationConfig FullyLruConfig() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1 = {"L1", 64, 16, 4, MappingKind::FullyAssociative, ReplacementKind::Lru};
    config.l2 = {"L2", 128, 16, 8, MappingKind::FullyAssociative, ReplacementKind::Lru};
    return config;
}

void RequireInvalidComparison(
    const std::vector<ComparisonPlan>& plans,
    const std::vector<MemoryAccess>& trace,
    const std::string& expectedMessage) {
    try {
        static_cast<void>(ComparisonRunner::Run(plans, trace));
    } catch (const std::invalid_argument& error) {
        Require(std::string(error.what()).find(expectedMessage) != std::string::npos,
                "Invalid comparison should explain the rejected input.");
        return;
    }
    throw std::runtime_error("Invalid comparison plans should be rejected.");
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
    const auto reversed = ComparisonRunner::Run({plans[1], plans[0]}, trace);
    Require(first.size() == 2 && second.size() == 2, "Runner should return one result per plan.");
    Require(first[0].statistics.accesses == trace.size() && first[1].statistics.accesses == trace.size(),
            "Every plan must receive the whole, identical trace.");
    Require(first[0].statistics.accesses == second[0].statistics.accesses &&
                first[0].statistics.memoryMisses == second[0].statistics.memoryMisses &&
                first[1].statistics.memoryMisses == second[1].statistics.memoryMisses,
            "Every run must start with independent simulator state.");
    Require(first[0].statistics.memoryMisses == reversed[1].statistics.memoryMisses &&
                first[1].statistics.memoryMisses == reversed[0].statistics.memoryMisses,
            "Changing plan execution order must not change results.");
}

void TestRejectsInvalidPlanCollections() {
    const std::vector<MemoryAccess> trace = {{0x00, false}};
    RequireInvalidComparison({{"Only one", DirectConfig()}}, trace, "two or three plans");
    RequireInvalidComparison(
        {{"One", DirectConfig()}, {"Two", SetLruConfig()}, {"Three", FullyLruConfig()},
         {"Four", DirectConfig()}},
        trace,
        "two or three plans");
    RequireInvalidComparison(
        {{"   ", DirectConfig()}, {"Valid", SetLruConfig()}}, trace, "needs a name");
    RequireInvalidComparison(
        {{"Duplicate", DirectConfig()}, {" Duplicate ", SetLruConfig()}}, trace, "must be distinct");

    auto invalidConfig = DirectConfig();
    invalidConfig.l1.associativity = 2;
    RequireInvalidComparison(
        {{"Invalid direct plan", invalidConfig}, {"Valid", SetLruConfig()}},
        trace,
        "Plan 'Invalid direct plan' is invalid");
}

void TestTeachingTraceDistinguishesMappingStrategies() {
    const std::vector<MemoryAccess> trace = {
        {0x00, false}, {0x80, false}, {0x00, false}, {0x80, false},
        {0x40, false}, {0x00, false}, {0x80, false}, {0x00, false},
    };
    const auto results = ComparisonRunner::Run(
        {{"Direct + FIFO", DirectConfig()},
         {"2-way Set + LRU", SetLruConfig()},
         {"Fully Associative + LRU", FullyLruConfig()}},
        trace);

    Require(results.size() == 3, "Teaching comparison should return all three strategies.");
    Require(results[0].statistics.accesses == trace.size() &&
                results[1].statistics.accesses == trace.size() &&
                results[2].statistics.accesses == trace.size(),
            "Every teaching plan must use all accesses.");
    Require(results[0].statistics.memoryMisses > results[1].statistics.memoryMisses &&
                results[1].statistics.memoryMisses > results[2].statistics.memoryMisses,
            "Teaching trace should show Direct, Set and Fully with decreasing miss counts.");
}

}  // namespace

void AddComparisonTests(TestList& tests) {
    tests.push_back(
        {"Comparison: independent plans use the same trace", TestRunsIndependentPlansAgainstTheSameTrace});
    tests.push_back({"Comparison: plan validation", TestRejectsInvalidPlanCollections});
    tests.push_back(
        {"Comparison: teaching trace distinguishes strategies", TestTeachingTraceDistinguishesMappingStrategies});
}

}  // namespace b5cache::tests
