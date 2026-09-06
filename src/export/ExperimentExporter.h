#pragma once

#include "common/CacheTypes.h"
#include "experiment/ComparisonRunner.h"

#include <filesystem>
#include <string>
#include <vector>

namespace b5cache {

// Read-only snapshot of everything one experiment produced. The exporter never
// runs a simulator and never modifies the data it receives; the MFC UI only
// collects the data and the target path.
struct ExperimentExportData {
    SimulationConfig config;
    std::vector<MemoryAccess> trace;          // full parsed trace
    std::vector<AccessResult> accessResults;  // results of the executed prefix of trace
    StatisticsSnapshot statistics;            // final snapshot after the last executed access
    std::string exportTime;                   // local time, "YYYY-MM-DD HH:MM:SS"

    // Optional stage 05 comparison payload; empty when the caller only exports
    // a single experiment.
    std::vector<ComparisonPlan> comparisonPlans;
    std::vector<ComparisonResult> comparisonResults;
};

// Formats experiment data as UTF-8 CSV or TXT documents and writes them to
// disk. This module is pure C++ and must not depend on MFC.
class ExperimentExporter final {
public:
    // Single experiment export: metadata, configuration, trace, per-access
    // results, final statistics, optional comparison table and a short
    // objective conclusion.
    static std::string FormatExperimentCsv(const ExperimentExportData& data);
    static std::string FormatExperimentTxt(const ExperimentExportData& data);

    // Stage 05 comparison export: metadata, trace counts, the unified
    // comparison table and the objective conclusion.
    static std::string FormatComparisonCsv(const ExperimentExportData& data);
    static std::string FormatComparisonTxt(const ExperimentExportData& data);

    // Writes content as UTF-8 with a BOM so Windows Excel and Notepad detect
    // the encoding (see docs/tasks/06-result-export.md). Throws
    // std::runtime_error carrying the path and reason when the file cannot be
    // created or written; errors are never swallowed.
    static void WriteUtf8File(const std::filesystem::path& path, const std::string& content);

    // "YYYY-MM-DD HH:MM:SS" local time, used for the metadata section.
    static std::string CurrentTimeStamp();
    // "YYYYMMDD_HHMMSS" local time, used for default file names.
    static std::string FileTimeStamp();

    static constexpr const char* kProgramVersion =
        "B5CacheVisualizer Stage 06 (experiment result export)";
    static constexpr const char* kExperimentTitle = "B5 Cache Experiment Export";
    static constexpr const char* kComparisonTitle = "B5 Cache Strategy Comparison Export";
};

}  // namespace b5cache
