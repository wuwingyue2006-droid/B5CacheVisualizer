#include "TestSuites.h"

#include "core/CacheSimulator.h"
#include "export/ExperimentExporter.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace b5cache::tests {
namespace {

namespace fs = std::filesystem;

constexpr auto kNotFound = std::string::npos;

ExperimentExportData BuildSampleExperiment() {
    ExperimentExportData data;
    data.config.l1 = {"L1", 64, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.config.l2 = {"L2", 128, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.trace = {{0x00, false}, {0x00, false}, {0x40, false}, {0x80, true}};

    CacheSimulator simulator(data.config);
    for (const auto& access : data.trace) {
        data.accessResults.push_back(simulator.Access(access));
    }
    data.statistics = simulator.Statistics();
    data.exportTime = "2026-09-05 12:00:00";
    return data;
}

void TestExperimentCsvContainsConfigTraceAndStatistics() {
    const auto data = BuildSampleExperiment();
    const auto csv = ExperimentExporter::FormatExperimentCsv(data);

    Require(csv.find("B5 Cache Experiment Export") != kNotFound,
            "CSV should start with the experiment export title.");
    Require(csv.find("Exported at,2026-09-05 12:00:00") != kNotFound,
            "CSV should record the export time.");
    Require(csv.find("[Configuration]") != kNotFound &&
                csv.find("[Trace Summary]") != kNotFound &&
                csv.find("[Trace]") != kNotFound &&
                csv.find("[Per-Access Results]") != kNotFound &&
                csv.find("[Final Statistics]") != kNotFound,
            "CSV should contain every required section.");
    Require(csv.find("L1,L1,64,16,4,1,Direct,FIFO") != kNotFound,
            "CSV should contain the full L1 configuration including the line count.");
    Require(csv.find("L2,L2,128,16,8,1,Direct,FIFO") != kNotFound,
            "CSV should contain the full L2 configuration including the line count.");
    Require(csv.find("Total accesses,4") != kNotFound &&
                csv.find("Read accesses,3") != kNotFound &&
                csv.find("Write accesses,1") != kNotFound,
            "CSV should report trace totals and read/write counts.");
    Require(csv.find("0x40") != kNotFound && csv.find("0x80") != kNotFound,
            "CSV should use the 0x hexadecimal address format.");
    Require(csv.find("L1 Hit") != kNotFound,
            "CSV should list per-access outcomes such as L1 Hit.");
    Require(csv.find("Overall hit rate") != kNotFound,
            "CSV should contain the final overall hit rate.");
}

void TestCsvNeverPrintsInvalidLineIndexAndMarksSkippedL2() {
    ExperimentExportData data;
    data.config.l1 = {"L1", 64, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.config.l2 = {"L2", 128, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.trace = {{0x00, false}, {0x00, false}};
    data.exportTime = "2026-09-05 12:00:00";

    AccessResult memoryMiss;
    memoryMiss.request = {0x00, false};
    memoryMiss.outcome = AccessOutcome::MemoryMiss;
    memoryMiss.l1 = {false, 0, 0, false, 0};
    memoryMiss.l2 = {false, 0, 0, false, 0};

    AccessResult l1Hit;
    l1Hit.request = {0x00, false};
    l1Hit.outcome = AccessOutcome::L1Hit;
    l1Hit.l1 = {true, 0, 0, false, 0};

    AccessResult evictingMiss;
    evictingMiss.request = {0xC0, false};
    evictingMiss.outcome = AccessOutcome::MemoryMiss;
    evictingMiss.l1 = {false, 0, 0, true, 3};
    evictingMiss.l2 = {false, 4, 4, false, 0};

    data.accessResults = {memoryMiss, l1Hit, evictingMiss};
    data.statistics.accesses = 3;
    data.statistics.reads = 3;

    const auto csv = ExperimentExporter::FormatExperimentCsv(data);

    Require(csv.find("18446744073709551615") == kNotFound,
            "The unsigned maximum must never appear for invalid line indexes.");
    Require(csv.find("1,R,0x0,0,L1 Hit,yes,0,0,no,,no,,,,,") != kNotFound,
            "An L1 hit must leave the L2 columns empty and mark L2 as not accessed.");
    Require(csv.find("2,R,0xC0,192,Memory Miss,no,0,0,yes,3,yes,no,4,4,no,") != kNotFound,
            "Only levels that report an eviction should carry the evicted block.");
}

void TestExperimentTxtIsHumanReadable() {
    const auto data = BuildSampleExperiment();
    const auto txt = ExperimentExporter::FormatExperimentTxt(data);

    Require(txt.find("B5 Cache Experiment Export") != kNotFound,
            "TXT should start with the experiment export title.");
    Require(txt.find("Exported at : 2026-09-05 12:00:00") != kNotFound,
            "TXT should record the export time.");
    Require(txt.find("[L1 Configuration]") != kNotFound &&
                txt.find("[L2 Configuration]") != kNotFound,
            "TXT should describe both cache levels.");
    Require(txt.find("0x40") != kNotFound,
            "TXT should use the 0x hexadecimal address format.");
    Require(txt.find("not accessed (L1 hit)") != kNotFound,
            "TXT must clearly mark that L2 is skipped on an L1 hit.");
    Require(txt.find("Overall hit rate") != kNotFound,
            "TXT should contain the final statistics.");
    Require(txt.find("18446744073709551615") == kNotFound,
            "The unsigned maximum must never appear in the TXT export.");
    Require(txt.find("100.00%") != kNotFound || txt.find("75.00%") != kNotFound,
            "TXT percentages should use the same precision as the UI.");
}

void TestComparisonCsvEscapesSpecialCharacters() {
    ExperimentExportData data;
    data.config.l1 = {"L1", 64, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.config.l2 = {"L2", 128, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.trace = {{0x00, false}, {0x80, false}, {0x00, false}, {0x80, false}};
    data.exportTime = "2026-09-05 12:00:00";

    const std::vector<ComparisonPlan> plans = {
        {"Plan, \"with comma\"", data.config},
        {"Second plan", data.config},
    };
    const auto results = ComparisonRunner::Run(plans, data.trace);
    data.comparisonPlans = plans;
    data.comparisonResults = results;

    const auto csv = ExperimentExporter::FormatComparisonCsv(data);
    Require(csv.find("\"Plan, \"\"with comma\"\"\"") != kNotFound,
            "Plan names containing commas or quotes must be escaped.");

    const auto txt = ExperimentExporter::FormatComparisonTxt(data);
    Require(txt.find("Plan, \"with comma\"") != kNotFound,
            "The TXT export should keep the plan name readable.");
    Require(txt.find("Highest overall hit rate") != kNotFound,
            "The comparison export should end with an objective conclusion.");
}

void TestComparisonExportRanksEveryPlan() {
    ExperimentExportData data;
    data.config.l1 = {"L1", 64, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.config.l2 = {"L2", 128, 16, 1, MappingKind::Direct, ReplacementKind::Fifo};
    data.trace = {
        {0x00, false}, {0x80, false}, {0x00, false}, {0x80, false},
        {0x40, false}, {0x00, false}, {0x80, false}, {0x00, false},
    };
    data.exportTime = "2026-09-05 12:00:00";

    const std::vector<ComparisonPlan> plans = {
        {"Direct + FIFO", data.config},
        {"2-way Set + LRU",
            SimulationConfig{
                {"L1", 64, 16, 2, MappingKind::SetAssociative, ReplacementKind::Lru},
                {"L2", 128, 16, 2, MappingKind::SetAssociative, ReplacementKind::Lru}}},
    };
    data.comparisonPlans = plans;
    data.comparisonResults = ComparisonRunner::Run(plans, data.trace);

    double best = -1.0;
    std::string bestPlan;
    for (const auto& result : data.comparisonResults) {
        if (result.statistics.OverallHitRate() > best) {
            best = result.statistics.OverallHitRate();
            bestPlan = result.planName;
        }
    }

    const auto csv = ExperimentExporter::FormatComparisonCsv(data);
    Require(csv.find("Highest overall hit rate: " + bestPlan) != kNotFound,
            "The CSV conclusion should name the plan with the highest overall hit rate.");
    Require(csv.find(bestPlan + ",64,16,2,SetAssociative,LRU,128,16,2,SetAssociative,LRU,") != kNotFound,
            "Each CSV comparison row should embed the full plan configuration.");

    const auto txt = ExperimentExporter::FormatComparisonTxt(data);
    Require(txt.find("Highest overall hit rate: " + bestPlan) != kNotFound,
            "The TXT conclusion should name the plan with the highest overall hit rate.");
    Require(txt.find("[Trace Summary]") != kNotFound,
            "The comparison TXT should report the shared trace size.");
}

void TestWriteUtf8FileUsesBomAndPropagatesErrors() {
    const auto directory = fs::temp_directory_path();
    const auto path = directory / (
        "b5cache_export_test_" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".csv");

    try {
        ExperimentExporter::WriteUtf8File(path, "A,B\r\n1,2\r\n");

        std::ifstream file(path, std::ios::binary);
        const std::string content(
            (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        Require(content.size() > 3 && content.compare(0, 3, "\xEF\xBB\xBF") == 0,
                "Exported files must start with the UTF-8 BOM.");
        Require(content.substr(3) == "A,B\r\n1,2\r\n",
                "The file content after the BOM must match the formatted text.");

        bool threw = false;
        try {
            ExperimentExporter::WriteUtf8File(
                directory / "b5cache_missing_directory_for_tests" / "out.csv", "x");
        } catch (const std::runtime_error& error) {
            threw = true;
            Require(std::string(error.what()).find("b5cache_missing_directory_for_tests") != kNotFound,
                    "The error message should contain the failing path.");
        }
        Require(threw, "File write failures must propagate as exceptions.");
    } catch (...) {
        fs::remove(path);
        throw;
    }
    fs::remove(path);
}

}  // namespace

void AddExportTests(TestList& tests) {
    tests.push_back({"Export: experiment CSV contains config, trace and statistics",
                     TestExperimentCsvContainsConfigTraceAndStatistics});
    tests.push_back({"Export: invalid line index and skipped L2", TestCsvNeverPrintsInvalidLineIndexAndMarksSkippedL2});
    tests.push_back({"Export: experiment TXT is human readable", TestExperimentTxtIsHumanReadable});
    tests.push_back({"Export: comparison CSV escaping", TestComparisonCsvEscapesSpecialCharacters});
    tests.push_back({"Export: comparison export ranks every plan", TestComparisonExportRanksEveryPlan});
    tests.push_back({"Export: UTF-8 BOM writing and error propagation", TestWriteUtf8FileUsesBomAndPropagatesErrors});
}

}  // namespace b5cache::tests
