#include "TestSuites.h"

#include "../B5CacheVisualizer/VisualizationController.h"
#include "core/CacheSimulator.h"
#include "experiment/ComparisonRunner.h"
#include "export/ExperimentExporter.h"
#include "trace/MemoryTraceParser.h"

#include <cmath>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace b5cache::tests {
namespace {

struct MatrixCase {
    std::string name;
    MappingKind l1Mapping;
    MappingKind l2Mapping;
    ReplacementKind replacement;
};

CacheLevelConfig MakeLevel(
    const std::string& name,
    const std::size_t sizeBytes,
    const MappingKind mapping,
    const ReplacementKind replacement) {
    constexpr std::size_t blockSize = 16;
    const std::size_t lineCount = sizeBytes / blockSize;
    std::size_t associativity = 1;
    if (mapping == MappingKind::SetAssociative) {
        associativity = 2;
    } else if (mapping == MappingKind::FullyAssociative) {
        associativity = lineCount;
    }
    return {name, sizeBytes, blockSize, associativity, mapping, replacement};
}

SimulationConfig MakeConfig(const MatrixCase& item) {
    return {
        MakeLevel("L1", 64, item.l1Mapping, item.replacement),
        MakeLevel("L2", 128, item.l2Mapping, item.replacement),
    };
}

std::vector<MatrixCase> ConfigurationMatrix() {
    return {
        {"Direct/Direct + FIFO", MappingKind::Direct, MappingKind::Direct, ReplacementKind::Fifo},
        {"Direct/Direct + LRU", MappingKind::Direct, MappingKind::Direct, ReplacementKind::Lru},
        {"Set/Set + FIFO", MappingKind::SetAssociative, MappingKind::SetAssociative, ReplacementKind::Fifo},
        {"Set/Set + LRU", MappingKind::SetAssociative, MappingKind::SetAssociative, ReplacementKind::Lru},
        {"Fully/Fully + FIFO", MappingKind::FullyAssociative, MappingKind::FullyAssociative, ReplacementKind::Fifo},
        {"Fully/Fully + LRU", MappingKind::FullyAssociative, MappingKind::FullyAssociative, ReplacementKind::Lru},
        {"Direct/Set + LRU", MappingKind::Direct, MappingKind::SetAssociative, ReplacementKind::Lru},
        {"Set/Fully + LRU", MappingKind::SetAssociative, MappingKind::FullyAssociative, ReplacementKind::Lru},
    };
}

std::vector<MemoryAccess> MakeStressTrace(const std::size_t count) {
    std::vector<MemoryAccess> trace;
    trace.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        const std::uint64_t block = (index * 17 + index / 7) % 64;
        trace.push_back({block * 16, index % 5 == 0});
    }
    return trace;
}

void RequireStatisticsConsistent(
    const StatisticsSnapshot& statistics,
    const std::size_t expectedAccesses,
    const std::size_t expectedWrites,
    const std::string& label) {
    Require(statistics.accesses == expectedAccesses, label + " should count every access.");
    Require(statistics.writes == expectedWrites, label + " should count writes exactly.");
    Require(statistics.reads + statistics.writes == statistics.accesses,
            label + " read/write totals should equal accesses.");
    Require(statistics.l1Hits + statistics.l2Hits + statistics.memoryMisses == statistics.accesses,
            label + " outcome totals should equal accesses.");
    for (const double rate : {
             statistics.L1HitRate(), statistics.L2HitRate(),
             statistics.OverallHitRate(), statistics.MissRate()}) {
        Require(std::isfinite(rate) && rate >= 0.0 && rate <= 1.0,
                label + " rates should remain finite and bounded.");
    }
}

void RunMatrixCase(const MatrixCase& item) {
    const auto trace = MakeStressTrace(2048);
    CacheSimulator simulator(MakeConfig(item));
    const auto results = simulator.Run(trace);

    Require(results.size() == trace.size(), item.name + " should return one result per access.");
    for (std::size_t index = 0; index < results.size(); ++index) {
        const auto& result = results[index];
        Require(result.request.address == trace[index].address &&
                    result.request.isWrite == trace[index].isWrite,
                item.name + " should preserve request order.");
        Require(result.l1.lineIndex != kInvalidIndex,
                item.name + " should always identify the accessed L1 line.");
        if (result.outcome == AccessOutcome::L1Hit) {
            Require(result.l2.lineIndex == kInvalidIndex,
                    item.name + " should leave L2 untouched on an L1 hit.");
        } else {
            Require(result.l2.lineIndex != kInvalidIndex,
                    item.name + " should identify the accessed L2 line after an L1 miss.");
        }
    }
    RequireStatisticsConsistent(simulator.Statistics(), trace.size(), 410, item.name);
}

void TestInvalidConfigurationMatrix() {
    const auto baseline = CacheSimulator::DefaultConfig();
    struct InvalidCase {
        std::string name;
        SimulationConfig config;
        std::string expectedLevel;
    };
    std::vector<InvalidCase> cases;

    auto addL1 = [&](const std::string& name, const CacheLevelConfig& level) {
        auto config = baseline;
        config.l1 = level;
        cases.push_back({name, config, "L1"});
    };
    addL1("zero size", {"L1", 0, 16, 1, MappingKind::Direct, ReplacementKind::Fifo});
    addL1("zero block", {"L1", 64, 0, 1, MappingKind::Direct, ReplacementKind::Fifo});
    addL1("indivisible size", {"L1", 65, 16, 1, MappingKind::Direct, ReplacementKind::Fifo});
    addL1("zero ways", {"L1", 64, 16, 0, MappingKind::Direct, ReplacementKind::Fifo});
    addL1("ways do not divide lines", {"L1", 64, 16, 3, MappingKind::SetAssociative, ReplacementKind::Fifo});
    addL1("direct with multiple ways", {"L1", 64, 16, 2, MappingKind::Direct, ReplacementKind::Fifo});
    addL1("fully with multiple sets", {"L1", 64, 16, 2, MappingKind::FullyAssociative, ReplacementKind::Fifo});
    addL1("set with one way", {"L1", 64, 16, 1, MappingKind::SetAssociative, ReplacementKind::Fifo});
    addL1("set with all ways", {"L1", 64, 16, 4, MappingKind::SetAssociative, ReplacementKind::Fifo});

    auto invalidL2 = baseline;
    invalidL2.l2 = {"L2", 128, 16, 3, MappingKind::SetAssociative, ReplacementKind::Lru};
    cases.push_back({"invalid L2", invalidL2, "L2"});

    for (const auto& item : cases) {
        bool threw = false;
        try {
            CacheSimulator simulator(item.config);
            static_cast<void>(simulator);
        } catch (const std::invalid_argument& error) {
            threw = true;
            Require(std::string(error.what()).find(item.expectedLevel) != std::string::npos,
                    item.name + " should identify the invalid cache level.");
        }
        Require(threw, item.name + " should be rejected.");
    }
}

std::string MakeTraceText(const std::size_t count) {
    std::ostringstream text;
    for (std::size_t index = 0; index < count; ++index) {
        if (index % 3 == 0) {
            text << "W 0x" << std::hex << std::uppercase << index * 16 << std::dec;
        } else if (index % 3 == 1) {
            text << "R " << index * 16;
        } else {
            text << index * 16;
        }
        if (index % 97 == 0) {
            text << " # stability sample";
        }
        text << "\n";
    }
    return text.str();
}

void ParseAndRunLargeTrace(const std::size_t count) {
    const auto trace = MemoryTraceParser::ParseText(MakeTraceText(count));
    Require(trace.size() == count, "Large trace parser should preserve every request.");
    Require(trace.back().address == (count - 1) * 16,
            "Large trace parser should preserve the final address.");

    CacheSimulator simulator;
    const auto results = simulator.Run(trace);
    Require(results.size() == count, "Large trace run should return every result.");
    const std::size_t expectedWrites = (count + 2) / 3;
    RequireStatisticsConsistent(simulator.Statistics(), count, expectedWrites, "Large trace");
}

void TestTraceWithOneThousandRequests() {
    ParseAndRunLargeTrace(1000);
}

void TestTraceWithTenThousandRequests() {
    ParseAndRunLargeTrace(10000);
}

bool SameDetail(const LevelAccessDetail& left, const LevelAccessDetail& right) {
    return left.hit == right.hit &&
        left.setIndex == right.setIndex &&
        left.lineIndex == right.lineIndex &&
        left.evicted == right.evicted &&
        left.evictedBlock == right.evictedBlock;
}

void TestResetProducesDeterministicReplay() {
    const auto trace = MakeStressTrace(4096);
    const MatrixCase item{
        "Set/Set + LRU", MappingKind::SetAssociative,
        MappingKind::SetAssociative, ReplacementKind::Lru};
    CacheSimulator simulator(MakeConfig(item));
    const auto first = simulator.Run(trace);
    const auto firstStatistics = simulator.Statistics();

    simulator.Reset();
    const auto second = simulator.Run(trace);
    const auto secondStatistics = simulator.Statistics();

    Require(first.size() == second.size(), "Reset replay should keep result count stable.");
    for (std::size_t index = 0; index < first.size(); ++index) {
        Require(first[index].outcome == second[index].outcome &&
                    SameDetail(first[index].l1, second[index].l1) &&
                    SameDetail(first[index].l2, second[index].l2),
                "Reset replay should reproduce every access detail.");
    }
    Require(firstStatistics.accesses == secondStatistics.accesses &&
                firstStatistics.l1Hits == secondStatistics.l1Hits &&
                firstStatistics.l2Hits == secondStatistics.l2Hits &&
                firstStatistics.memoryMisses == secondStatistics.memoryMisses,
            "Reset replay should reproduce final statistics.");
}

void TestComparisonStressMatchesStandaloneRuns() {
    const auto trace = MakeStressTrace(10000);
    const auto matrix = ConfigurationMatrix();
    const std::vector<ComparisonPlan> plans = {
        {matrix[0].name, MakeConfig(matrix[0])},
        {matrix[3].name, MakeConfig(matrix[3])},
        {matrix[5].name, MakeConfig(matrix[5])},
    };
    const auto comparison = ComparisonRunner::Run(plans, trace);
    Require(comparison.size() == plans.size(), "Comparison stress should return every plan.");

    for (std::size_t index = 0; index < plans.size(); ++index) {
        CacheSimulator standalone(plans[index].config);
        standalone.Run(trace);
        const auto expected = standalone.Statistics();
        const auto actual = comparison[index].statistics;
        Require(actual.accesses == expected.accesses &&
                    actual.l1Hits == expected.l1Hits &&
                    actual.l2Hits == expected.l2Hits &&
                    actual.memoryMisses == expected.memoryMisses,
                plans[index].name + " comparison result should match a standalone run.");
    }
}

void TestLargeExportRemainsComplete() {
    ExperimentExportData data;
    data.config = CacheSimulator::DefaultConfig();
    data.trace = MakeStressTrace(10000);
    CacheSimulator simulator(data.config);
    data.accessResults = simulator.Run(data.trace);
    data.statistics = simulator.Statistics();
    data.exportTime = "2026-09-06 12:00:00";

    const auto csv = ExperimentExporter::FormatExperimentCsv(data);
    Require(csv.find("Total accesses,10000") != std::string::npos,
            "Large export should report the full trace size.");
    Require(csv.find("9999,") != std::string::npos,
            "Large export should contain the final trace and result rows.");
    Require(csv.find("[Final Statistics]") != std::string::npos &&
                csv.find("[Conclusion]") != std::string::npos,
            "Large export should remain structurally complete.");
}

b5cacheui::VisualizationFrame MakeFrame(const std::uint64_t address) {
    b5cacheui::VisualizationFrame frame;
    frame.result.request.address = address;
    return frame;
}

void TestVisualizationControllerContinuousOperations() {
    b5cacheui::VisualizationController controller;
    Require(controller.Current() == nullptr && controller.FrameCount() == 0 &&
                controller.State() == b5cacheui::PlaybackState::Stopped,
            "Controller should begin empty and stopped.");

    controller.Append(MakeFrame(0x00));
    controller.Append(MakeFrame(0x10));
    controller.Append(MakeFrame(0x20));
    Require(controller.FrameCount() == 3 && controller.CurrentPosition() == 3,
            "Append should advance to the latest frame.");
    Require(controller.Frames()[0].frameNumber == 1 && controller.Frames()[2].frameNumber == 3,
            "Frame numbers should be stable and one-based.");

    Require(controller.MovePrevious() && controller.MovePrevious() && !controller.MovePrevious(),
            "Previous should stop safely at the first frame.");
    bool appendRejected = false;
    try {
        controller.Append(MakeFrame(0x30));
    } catch (const std::logic_error&) {
        appendRejected = true;
    }
    Require(appendRejected, "Appending while reviewing history should be rejected.");
    Require(controller.MoveNext() && controller.MoveNext() && !controller.MoveNext(),
            "Next should stop safely at the latest recorded frame.");

    controller.Start();
    controller.Pause();
    Require(controller.State() == b5cacheui::PlaybackState::Paused,
            "Pause should preserve the current position.");
    controller.Start();
    controller.Stop();
    Require(controller.State() == b5cacheui::PlaybackState::Stopped &&
                controller.CurrentPosition() == 3,
            "Stop should preserve the current frame.");

    controller.SetSpeed(b5cacheui::PlaybackSpeed::Slow);
    const auto slow = controller.TimerIntervalMs();
    controller.SetSpeed(b5cacheui::PlaybackSpeed::Normal);
    const auto normal = controller.TimerIntervalMs();
    controller.SetSpeed(b5cacheui::PlaybackSpeed::Fast);
    const auto fast = controller.TimerIntervalMs();
    Require(slow > normal && normal > fast,
            "Playback speed intervals should remain strictly ordered.");

    controller.Reset();
    Require(controller.Current() == nullptr && controller.FrameCount() == 0 &&
                controller.CurrentPosition() == 0 &&
                controller.State() == b5cacheui::PlaybackState::Stopped,
            "Reset should clear history, position and playback state.");
}

}  // namespace

void AddStabilityTests(TestList& tests) {
    for (const auto& item : ConfigurationMatrix()) {
        tests.push_back({"Stability matrix: " + item.name, [item]() { RunMatrixCase(item); }});
    }
    tests.push_back({"Stability: invalid configuration matrix", TestInvalidConfigurationMatrix});
    tests.push_back({"Stability: parse and run 1000 requests", TestTraceWithOneThousandRequests});
    tests.push_back({"Stability: parse and run 10000 requests", TestTraceWithTenThousandRequests});
    tests.push_back({"Stability: reset produces deterministic replay", TestResetProducesDeterministicReplay});
    tests.push_back({"Stability: comparison stress matches standalone runs", TestComparisonStressMatchesStandaloneRuns});
    tests.push_back({"Stability: 10000-access export remains complete", TestLargeExportRemainsComplete});
    tests.push_back({"Stability: visualization controller continuous operations",
                     TestVisualizationControllerContinuousOperations});
}

}  // namespace b5cache::tests
