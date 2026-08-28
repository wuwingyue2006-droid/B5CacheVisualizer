#include "TestSuites.h"

#include "core/CacheSimulator.h"

#include <stdexcept>
#include <string>

namespace b5cache::tests {
namespace {

void RequireInvalidConfiguration(
    const SimulationConfig& config,
    const std::string& expectedLevel,
    const std::string& scenario) {
    try {
        const CacheSimulator simulator(config);
        static_cast<void>(simulator);
    } catch (const std::invalid_argument& error) {
        Require(std::string(error.what()).find(expectedLevel) != std::string::npos,
                scenario + " error should identify " + expectedLevel + ".");
        return;
    }
    throw std::runtime_error(scenario + " should be rejected during construction.");
}

const CacheLineState& FindBlock(const CacheLevel& level, const std::uint64_t blockNumber) {
    for (const auto& set : level.Sets()) {
        for (const auto& line : set) {
            if (line.valid && line.blockNumber == blockNumber) {
                return line;
            }
        }
    }
    throw std::runtime_error("Expected cache block was not found.");
}

bool SameLineState(const CacheLineState& left, const CacheLineState& right) {
    return left.valid == right.valid &&
        left.dirty == right.dirty &&
        left.tag == right.tag &&
        left.blockNumber == right.blockNumber &&
        left.insertedAt == right.insertedAt &&
        left.lastUsedAt == right.lastUsedAt;
}

void RequireSameCacheState(
    const std::vector<std::vector<CacheLineState>>& expected,
    const std::vector<std::vector<CacheLineState>>& actual,
    const std::string& message) {
    Require(expected.size() == actual.size(), message);
    for (std::size_t setIndex = 0; setIndex < expected.size(); ++setIndex) {
        Require(expected[setIndex].size() == actual[setIndex].size(), message);
        for (std::size_t lineIndex = 0; lineIndex < expected[setIndex].size(); ++lineIndex) {
            Require(SameLineState(expected[setIndex][lineIndex], actual[setIndex][lineIndex]), message);
        }
    }
}

void RequireCacheCleared(const CacheLevel& level, const std::string& message) {
    for (const auto& set : level.Sets()) {
        for (const auto& line : set) {
            Require(!line.valid && !line.dirty && line.tag == 0 && line.blockNumber == 0 &&
                        line.insertedAt == 0 && line.lastUsedAt == 0,
                    message);
        }
    }
}

void TestRejectsZeroSizeAndBlockSize() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1.sizeBytes = 0;
    RequireInvalidConfiguration(config, "L1", "Zero L1 size");

    config = CacheSimulator::DefaultConfig();
    config.l2.blockSizeBytes = 0;
    RequireInvalidConfiguration(config, "L2", "Zero L2 block size");
}

void TestRejectsIndivisibleSizes() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1.sizeBytes = 65;
    RequireInvalidConfiguration(config, "L1", "Indivisible L1 size");

    config = CacheSimulator::DefaultConfig();
    config.l2.sizeBytes = 129;
    RequireInvalidConfiguration(config, "L2", "Indivisible L2 size");
}

void TestRejectsInvalidAssociativity() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1.associativity = 0;
    RequireInvalidConfiguration(config, "L1", "Zero L1 associativity");

    config = CacheSimulator::DefaultConfig();
    config.l2.mapping = MappingKind::SetAssociative;
    config.l2.associativity = 3;
    RequireInvalidConfiguration(config, "L2", "L2 associativity that does not divide line count");
}

void TestDirectMappingConfigurationRules() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1.associativity = 2;
    RequireInvalidConfiguration(config, "L1", "Direct mapping with multiple ways");

    const CacheSimulator valid(CacheSimulator::DefaultConfig());
    Require(valid.L1().LineCount() == 4 && valid.L1().Sets().size() == 4,
            "Direct mapping with associativity 1 should be valid.");

    config = CacheSimulator::DefaultConfig();
    config.l1.mapping = static_cast<MappingKind>(99);
    RequireInvalidConfiguration(config, "L1", "Unknown L1 mapping kind");

    config = CacheSimulator::DefaultConfig();
    config.l2.replacement = static_cast<ReplacementKind>(99);
    RequireInvalidConfiguration(config, "L2", "Unknown L2 replacement kind");
}

void TestFullyAssociativeConfigurationRules() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1.mapping = MappingKind::FullyAssociative;
    config.l1.associativity = 2;
    RequireInvalidConfiguration(config, "L1", "Fully associative cache with multiple sets");

    config.l1.associativity = 4;
    const CacheSimulator valid(config);
    Require(valid.L1().Sets().size() == 1 && valid.L1().Sets()[0].size() == 4,
            "Fully associative configuration should contain one set with every line.");
}

void TestSetAssociativeConfigurationRules() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1.mapping = MappingKind::SetAssociative;
    config.l1.associativity = 1;
    RequireInvalidConfiguration(config, "L1", "Set associative cache with one way");

    config.l1.associativity = 4;
    RequireInvalidConfiguration(config, "L1", "Set associative cache with one set");

    config.l1.sizeBytes = 60;
    config.l1.blockSizeBytes = 10;
    config.l1.associativity = 2;
    const CacheSimulator valid(config);
    Require(valid.L1().LineCount() == 6 && valid.L1().Sets().size() == 3,
            "Valid non-power-of-two set associative configuration should be accepted.");
}

void TestMemoryMissFillsBothLevelsAndReportsDetails() {
    CacheSimulator simulator;
    const auto result = simulator.Access({0x10, false});

    Require(result.request.address == 0x10 && !result.request.isWrite,
            "AccessResult should preserve the original request.");
    Require(result.outcome == AccessOutcome::MemoryMiss, "First access should miss memory.");
    Require(!result.l1.hit && result.l1.setIndex == 1 && result.l1.lineIndex == 0 &&
                !result.l1.evicted && result.l1.evictedBlock == 0,
            "L1 miss insertion detail is incorrect.");
    Require(!result.l2.hit && result.l2.setIndex == 1 && result.l2.lineIndex == 0 &&
                !result.l2.evicted && result.l2.evictedBlock == 0,
            "L2 miss insertion detail is incorrect.");

    const auto& l1Line = FindBlock(simulator.L1(), 1);
    const auto& l2Line = FindBlock(simulator.L2(), 1);
    Require(!l1Line.dirty && l1Line.insertedAt == 1 && l1Line.lastUsedAt == 1,
            "Memory miss should fill a clean L1 line at the current tick.");
    Require(!l2Line.dirty && l2Line.insertedAt == 1 && l2Line.lastUsedAt == 1,
            "Memory miss should fill a clean L2 line at the same tick.");
}

void TestL1HitDoesNotAccessL2() {
    CacheSimulator simulator;
    simulator.Access({0x00, false});
    const auto l2Before = simulator.L2().Sets();

    const auto result = simulator.Access({0x00, true});
    Require(result.outcome == AccessOutcome::L1Hit, "Repeated access should hit L1.");
    Require(result.l1.hit && result.l1.setIndex == 0 && result.l1.lineIndex == 0 &&
                !result.l1.evicted,
            "L1 hit detail is incorrect.");
    Require(!result.l2.hit && result.l2.lineIndex == kInvalidIndex && !result.l2.evicted,
            "L1 hit should leave the L2 result untouched.");
    RequireSameCacheState(l2Before, simulator.L2().Sets(), "L1 hit must not modify any L2 line.");

    const auto& l1Line = FindBlock(simulator.L1(), 0);
    Require(l1Line.dirty && l1Line.insertedAt == 1 && l1Line.lastUsedAt == 2,
            "Write hit should mark L1 dirty and advance its hit timestamp once.");
}

void TestL2HitFillsTheCorrespondingL1Block() {
    auto config = CacheSimulator::DefaultConfig();
    config.l1 = {"L1", 32, 8, 1, MappingKind::Direct, ReplacementKind::Fifo};
    config.l2 = {"L2", 64, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    CacheSimulator simulator(config);

    simulator.Access({0x00, false});
    const auto result = simulator.Access({0x08, false});

    Require(result.outcome == AccessOutcome::L2Hit, "Address in an existing L2 block should hit L2.");
    Require(!result.l1.hit && result.l1.setIndex == 1 && result.l1.lineIndex == 0 &&
                !result.l1.evicted,
            "L2 hit should report the independently mapped L1 insertion.");
    Require(result.l2.hit && result.l2.setIndex == 0 && result.l2.lineIndex == 0 &&
                !result.l2.evicted,
            "L2 hit detail is incorrect.");
    Require(FindBlock(simulator.L1(), 1).insertedAt == 2,
            "L2 hit should fill the corresponding smaller L1 block.");
    Require(FindBlock(simulator.L2(), 0).lastUsedAt == 2,
            "L2 hit should update L2 at the same access tick.");
}

void TestWriteAccessesSetDirtyBits() {
    CacheSimulator writeMissSimulator;
    const auto writeMiss = writeMissSimulator.Access({0x20, true});
    Require(writeMiss.outcome == AccessOutcome::MemoryMiss,
            "First write should follow the write-allocate memory-miss path.");
    Require(FindBlock(writeMissSimulator.L1(), 2).dirty && FindBlock(writeMissSimulator.L2(), 2).dirty,
            "Write miss should mark both inserted lines dirty.");

    CacheSimulator l2HitSimulator;
    l2HitSimulator.Access({0x00, false});
    l2HitSimulator.Access({0x40, false});
    const auto writeL2Hit = l2HitSimulator.Access({0x00, true});
    Require(writeL2Hit.outcome == AccessOutcome::L2Hit, "Restored write should hit L2.");
    Require(FindBlock(l2HitSimulator.L1(), 0).dirty && FindBlock(l2HitSimulator.L2(), 0).dirty,
            "Write L2 hit should mark L2 and the inserted L1 line dirty.");
}

void TestEvictionDetailsForBothLevels() {
    CacheSimulator simulator;
    simulator.Access({0x00, false});
    const auto result = simulator.Access({0x80, true});

    Require(result.outcome == AccessOutcome::MemoryMiss, "Conflicting new block should miss memory.");
    Require(!result.l1.hit && result.l1.setIndex == 0 && result.l1.lineIndex == 0 &&
                result.l1.evicted && result.l1.evictedBlock == 0,
            "L1 eviction detail should identify block 0.");
    Require(!result.l2.hit && result.l2.setIndex == 0 && result.l2.lineIndex == 0 &&
                result.l2.evicted && result.l2.evictedBlock == 0,
            "L2 eviction detail should identify block 0.");
    Require(FindBlock(simulator.L1(), 8).dirty && FindBlock(simulator.L2(), 8).dirty,
            "Evicting write should install dirty block 8 in both levels.");
}

void TestRunOrderTickAndStatistics() {
    CacheSimulator simulator;
    const std::vector<MemoryAccess> trace = {
        {0x00, false},
        {0x00, false},
        {0x40, true},
        {0x00, false},
    };
    const auto results = simulator.Run(trace);

    Require(results.size() == trace.size(), "Run should return one result per request.");
    Require(results[0].request.address == trace[0].address &&
                results[1].request.address == trace[1].address &&
                results[2].request.address == trace[2].address &&
                results[3].request.address == trace[3].address,
            "Run should preserve trace order.");
    Require(results[0].outcome == AccessOutcome::MemoryMiss &&
                results[1].outcome == AccessOutcome::L1Hit &&
                results[2].outcome == AccessOutcome::MemoryMiss &&
                results[3].outcome == AccessOutcome::L2Hit,
            "Run outcomes should match the two-level access sequence.");

    const auto statistics = simulator.Statistics();
    Require(statistics.accesses == 4 && statistics.reads == 3 && statistics.writes == 1,
            "Each Access should update access and read/write statistics exactly once.");
    Require(statistics.l1Hits == 1 && statistics.l2Hits == 1 && statistics.memoryMisses == 2,
            "Each Access should record exactly one outcome.");

    const auto& l1Block0 = FindBlock(simulator.L1(), 0);
    const auto& l2Block0 = FindBlock(simulator.L2(), 0);
    const auto& l2Block4 = FindBlock(simulator.L2(), 4);
    Require(l1Block0.insertedAt == 4 && l1Block0.lastUsedAt == 4,
            "L2 hit should refill L1 using the fourth access tick.");
    Require(l2Block0.insertedAt == 1 && l2Block0.lastUsedAt == 4,
            "L2 block should retain insertion time and record the fourth access tick.");
    Require(l2Block4.insertedAt == 3 && l2Block4.lastUsedAt == 3,
            "Both levels should use one shared tick for the third access.");
}

void TestResetClearsEveryCacheLineField() {
    CacheSimulator simulator;
    simulator.Access({0x00, true});
    simulator.Access({0x00, false});
    simulator.Access({0x10, true});

    Require(FindBlock(simulator.L1(), 0).dirty && FindBlock(simulator.L1(), 0).lastUsedAt == 2,
            "Reset test should first create dirty L1 state and updated LRU time.");
    Require(FindBlock(simulator.L2(), 1).dirty && FindBlock(simulator.L2(), 1).insertedAt == 3,
            "Reset test should first create dirty L2 state and FIFO time.");

    simulator.Reset();

    RequireCacheCleared(simulator.L1(), "Reset should clear every L1 line field.");
    RequireCacheCleared(simulator.L2(), "Reset should clear every L2 line field.");
}

void TestResetRestartsTickAndStatistics() {
    CacheSimulator simulator;
    simulator.Access({0x00, true});
    simulator.Access({0x40, false});
    simulator.Reset();

    const auto statistics = simulator.Statistics();
    Require(statistics.accesses == 0 && statistics.reads == 0 && statistics.writes == 0 &&
                statistics.l1Hits == 0 && statistics.l2Hits == 0 && statistics.memoryMisses == 0,
            "Reset should clear every statistics counter.");
    Require(statistics.L1HitRate() == 0.0 && statistics.L2HitRate() == 0.0 &&
                statistics.OverallHitRate() == 0.0 && statistics.MissRate() == 0.0,
            "Reset statistics should report zero rates.");

    const auto firstAfterReset = simulator.Access({0x00, false});
    Require(firstAfterReset.outcome == AccessOutcome::MemoryMiss,
            "Reset should clear cache lines.");
    Require(FindBlock(simulator.L1(), 0).insertedAt == 1 &&
                FindBlock(simulator.L1(), 0).lastUsedAt == 1 &&
                FindBlock(simulator.L2(), 0).insertedAt == 1 &&
                FindBlock(simulator.L2(), 0).lastUsedAt == 1,
            "Reset should restart the access tick.");
    const auto restartedStatistics = simulator.Statistics();
    Require(restartedStatistics.accesses == 1 && restartedStatistics.reads == 1 &&
                restartedStatistics.writes == 0 && restartedStatistics.memoryMisses == 1,
            "Statistics should restart from the first post-reset access.");
}

}  // namespace

void AddCoreTests(TestList& tests) {
    tests.push_back({"Core A1: zero size and block size", TestRejectsZeroSizeAndBlockSize});
    tests.push_back({"Core A1: indivisible sizes", TestRejectsIndivisibleSizes});
    tests.push_back({"Core A1: invalid associativity", TestRejectsInvalidAssociativity});
    tests.push_back({"Core A1: direct mapping configuration", TestDirectMappingConfigurationRules});
    tests.push_back({"Core A1: fully associative configuration", TestFullyAssociativeConfigurationRules});
    tests.push_back({"Core A1: set associative configuration", TestSetAssociativeConfigurationRules});
    tests.push_back({"Core A2: memory miss details", TestMemoryMissFillsBothLevelsAndReportsDetails});
    tests.push_back({"Core A2: L1 hit skips L2", TestL1HitDoesNotAccessL2});
    tests.push_back({"Core A2: L2 hit fills L1", TestL2HitFillsTheCorrespondingL1Block});
    tests.push_back({"Core A2: write dirty state", TestWriteAccessesSetDirtyBits});
    tests.push_back({"Core A2: two-level eviction details", TestEvictionDetailsForBothLevels});
    tests.push_back({"Core A2: run order, tick and statistics", TestRunOrderTickAndStatistics});
    tests.push_back({"Core A3: reset clears all cache state", TestResetClearsEveryCacheLineField});
    tests.push_back({"Core A3: reset restarts tick and statistics", TestResetRestartsTickAndStatistics});
}

}  // namespace b5cache::tests
