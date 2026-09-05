#include "export/ExperimentExporter.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace b5cache {
namespace {

std::string EscapeCsvField(const std::string& field) {
    const bool needsQuoting = field.find_first_of(",\"\r\n") != std::string::npos;
    if (!needsQuoting) {
        return field;
    }

    std::string escaped = "\"";
    for (const char character : field) {
        if (character == '"') {
            escaped += "\"\"";
        } else {
            escaped += character;
        }
    }
    escaped += "\"";
    return escaped;
}

std::string CsvRow(const std::vector<std::string>& fields) {
    std::string row;
    for (std::size_t index = 0; index < fields.size(); ++index) {
        if (index != 0) {
            row += ",";
        }
        row += EscapeCsvField(fields[index]);
    }
    row += "\r\n";
    return row;
}

std::string PercentText(const double rate) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << rate * 100.0 << "%";
    return stream.str();
}

std::string HexAddressText(const std::uint64_t address) {
    std::ostringstream stream;
    stream << "0x" << std::hex << std::uppercase << address;
    return stream.str();
}

// An invalid line index must never be printed as the unsigned maximum value.
std::string LineIndexText(const std::size_t lineIndex) {
    return lineIndex == kInvalidIndex ? std::string{} : std::to_string(lineIndex);
}

std::string LineCountText(const CacheLevelConfig& config) {
    return config.blockSizeBytes == 0
        ? std::string("N/A")
        : std::to_string(config.sizeBytes / config.blockSizeBytes);
}

const char* AccessTypeName(const MemoryAccess& access) noexcept {
    return access.isWrite ? "W" : "R";
}

std::size_t CountReads(const std::vector<MemoryAccess>& trace) noexcept {
    std::size_t reads = 0;
    for (const auto& access : trace) {
        if (!access.isWrite) {
            ++reads;
        }
    }
    return reads;
}

std::size_t CountWrites(const std::vector<MemoryAccess>& trace) noexcept {
    return trace.size() - CountReads(trace);
}

std::uint64_t ExecutedAccessCount(const ExperimentExportData& data) noexcept {
    if (!data.accessResults.empty()) {
        return static_cast<std::uint64_t>(data.accessResults.size());
    }
    if (!data.comparisonResults.empty()) {
        return data.comparisonResults.front().statistics.accesses;
    }
    return 0;
}

std::string LevelDetailText(const std::string& level, const LevelAccessDetail& detail, const bool accessed) {
    if (!accessed) {
        return level + ": not accessed (L1 hit)";
    }

    std::string text = level + ": " + (detail.hit ? "hit" : "miss") +
        ", set " + std::to_string(detail.setIndex) + ", line ";
    text += detail.lineIndex == kInvalidIndex ? "N/A" : std::to_string(detail.lineIndex);
    if (detail.evicted) {
        text += ", evicted block " + std::to_string(detail.evictedBlock);
    }
    return text;
}

std::string AccessResultTxtRow(const std::size_t index, const AccessResult& result) {
    const bool l2Accessed = result.outcome != AccessOutcome::L1Hit;
    std::ostringstream stream;
    stream << std::right << std::setw(6) << index
           << "  " << std::left << std::setw(4) << AccessTypeName(result.request)
           << std::right << std::setw(12) << HexAddressText(result.request.address)
           << " (" << std::dec << result.request.address << ")"
           << "  " << std::left << std::setw(13) << ToString(result.outcome)
           << "  " << std::left << std::setw(36) << LevelDetailText("L1", result.l1, true)
           << "  " << LevelDetailText("L2", result.l2, l2Accessed);
    return stream.str();
}

std::string TraceTxtRow(const std::size_t index, const MemoryAccess& access) {
    std::ostringstream stream;
    stream << std::right << std::setw(6) << index
           << "  " << std::left << std::setw(4) << AccessTypeName(access)
           << std::right << std::setw(12) << HexAddressText(access.address)
           << " (" << std::dec << access.address << ")";
    return stream.str();
}

std::string TraceSummaryCsvRows(const ExperimentExportData& data) {
    std::string rows;
    rows += CsvRow({"Item", "Value"});
    rows += CsvRow({"Total accesses", std::to_string(data.trace.size())});
    rows += CsvRow({"Read accesses", std::to_string(CountReads(data.trace))});
    rows += CsvRow({"Write accesses", std::to_string(CountWrites(data.trace))});
    rows += CsvRow({"Executed accesses", std::to_string(ExecutedAccessCount(data))});
    return rows;
}

void AppendTraceSummaryTxt(std::string& text, const ExperimentExportData& data) {
    text += "[Trace Summary]\r\n";
    text += "  Total accesses    : " + std::to_string(data.trace.size()) + "\r\n";
    text += "  Read accesses     : " + std::to_string(CountReads(data.trace)) + "\r\n";
    text += "  Write accesses    : " + std::to_string(CountWrites(data.trace)) + "\r\n";
    text += "  Executed accesses : " + std::to_string(ExecutedAccessCount(data)) + "\r\n";
}

std::string TraceCsvRows(const std::vector<MemoryAccess>& trace) {
    std::string rows;
    rows += CsvRow({"Index", "Type", "Address (hex)", "Address (decimal)"});
    for (std::size_t index = 0; index < trace.size(); ++index) {
        const auto& access = trace[index];
        rows += CsvRow({
            std::to_string(index),
            AccessTypeName(access),
            HexAddressText(access.address),
            std::to_string(access.address)});
    }
    return rows;
}

void AppendTraceTxt(std::string& text, const std::vector<MemoryAccess>& trace) {
    text += "[Trace]\r\n";
    text += "  Index  Type  Address\r\n";
    for (std::size_t index = 0; index < trace.size(); ++index) {
        text += "  " + TraceTxtRow(index, trace[index]) + "\r\n";
    }
}

std::string ConfigurationCsvRows(const SimulationConfig& config) {
    std::string rows;
    rows += CsvRow({
        "Level", "Name", "Size (bytes)", "Block size (bytes)",
        "Line count", "Associativity", "Mapping", "Replacement"});

    std::size_t levelIndex = 0;
    for (const auto* level : {&config.l1, &config.l2}) {
        const char* levelName = levelIndex == 0 ? "L1" : "L2";
        rows += CsvRow({
            levelName,
            level->name,
            std::to_string(level->sizeBytes),
            std::to_string(level->blockSizeBytes),
            LineCountText(*level),
            std::to_string(level->associativity),
            ToString(level->mapping),
            ToString(level->replacement)});
        ++levelIndex;
    }
    return rows;
}

void AppendLevelConfigTxt(std::string& text, const char* level, const CacheLevelConfig& config) {
    text += std::string("[") + level + " Configuration]\r\n";
    text += "  Name          : " + config.name + "\r\n";
    text += "  Size          : " + std::to_string(config.sizeBytes) + " bytes\r\n";
    text += "  Block size    : " + std::to_string(config.blockSizeBytes) + " bytes\r\n";
    text += "  Line count    : " + LineCountText(config) + "\r\n";
    text += "  Associativity : " + std::to_string(config.associativity) + "\r\n";
    text += "  Mapping       : " + std::string(ToString(config.mapping)) + "\r\n";
    text += "  Replacement   : " + std::string(ToString(config.replacement)) + "\r\n";
}

std::string FinalStatisticsCsvRows(const StatisticsSnapshot& snapshot) {
    std::string rows;
    rows += CsvRow({"Metric", "Value"});
    rows += CsvRow({"Accesses", std::to_string(snapshot.accesses)});
    rows += CsvRow({"Reads", std::to_string(snapshot.reads)});
    rows += CsvRow({"Writes", std::to_string(snapshot.writes)});
    rows += CsvRow({"L1 hits", std::to_string(snapshot.l1Hits)});
    rows += CsvRow({"L2 hits", std::to_string(snapshot.l2Hits)});
    rows += CsvRow({"Memory misses", std::to_string(snapshot.memoryMisses)});
    rows += CsvRow({"L1 hit rate", PercentText(snapshot.L1HitRate())});
    rows += CsvRow({"L2 hit rate", PercentText(snapshot.L2HitRate())});
    rows += CsvRow({"Overall hit rate", PercentText(snapshot.OverallHitRate())});
    rows += CsvRow({"Miss rate", PercentText(snapshot.MissRate())});
    return rows;
}

void AppendFinalStatisticsTxt(std::string& text, const StatisticsSnapshot& snapshot) {
    text += "[Final Statistics]\r\n";
    text += "  Accesses         : " + std::to_string(snapshot.accesses) + "\r\n";
    text += "  Reads            : " + std::to_string(snapshot.reads) + "\r\n";
    text += "  Writes           : " + std::to_string(snapshot.writes) + "\r\n";
    text += "  L1 hits          : " + std::to_string(snapshot.l1Hits) + "\r\n";
    text += "  L2 hits          : " + std::to_string(snapshot.l2Hits) + "\r\n";
    text += "  Memory misses    : " + std::to_string(snapshot.memoryMisses) + "\r\n";
    text += "  L1 hit rate      : " + PercentText(snapshot.L1HitRate()) + "\r\n";
    text += "  L2 hit rate      : " + PercentText(snapshot.L2HitRate()) + "\r\n";
    text += "  Overall hit rate : " + PercentText(snapshot.OverallHitRate()) + "\r\n";
    text += "  Miss rate        : " + PercentText(snapshot.MissRate()) + "\r\n";
}

struct LevelConfigColumns {
    std::string size;
    std::string block;
    std::string associativity;
    std::string mapping;
    std::string replacement;
};

LevelConfigColumns LevelConfigColumnsFor(const CacheLevelConfig* level) {
    if (level == nullptr) {
        return {"N/A", "N/A", "N/A", "N/A", "N/A"};
    }
    return {
        std::to_string(level->sizeBytes),
        std::to_string(level->blockSizeBytes),
        std::to_string(level->associativity),
        ToString(level->mapping),
        ToString(level->replacement)};
}

std::string ComparisonCsvTable(const ExperimentExportData& data) {
    std::string table;
    table += CsvRow({
        "Plan",
        "L1 Size (bytes)", "L1 Block (bytes)", "L1 Associativity", "L1 Mapping", "L1 Replacement",
        "L2 Size (bytes)", "L2 Block (bytes)", "L2 Associativity", "L2 Mapping", "L2 Replacement",
        "Accesses", "Reads", "Writes", "L1 Hits", "L2 Hits", "Memory Misses",
        "L1 Hit Rate", "L2 Hit Rate", "Overall Hit Rate", "Miss Rate"});

    for (const auto& result : data.comparisonResults) {
        const SimulationConfig* planConfig = nullptr;
        for (const auto& plan : data.comparisonPlans) {
            if (plan.name == result.planName) {
                planConfig = &plan.config;
                break;
            }
        }

        const LevelConfigColumns l1 = LevelConfigColumnsFor(planConfig == nullptr ? nullptr : &planConfig->l1);
        const LevelConfigColumns l2 = LevelConfigColumnsFor(planConfig == nullptr ? nullptr : &planConfig->l2);
        const auto& statistics = result.statistics;

        table += CsvRow({
            result.planName,
            l1.size, l1.block, l1.associativity, l1.mapping, l1.replacement,
            l2.size, l2.block, l2.associativity, l2.mapping, l2.replacement,
            std::to_string(statistics.accesses),
            std::to_string(statistics.reads),
            std::to_string(statistics.writes),
            std::to_string(statistics.l1Hits),
            std::to_string(statistics.l2Hits),
            std::to_string(statistics.memoryMisses),
            PercentText(statistics.L1HitRate()),
            PercentText(statistics.L2HitRate()),
            PercentText(statistics.OverallHitRate()),
            PercentText(statistics.MissRate())});
    }
    return table;
}

std::string ComparisonTxtTable(const ExperimentExportData& data) {
    std::string table;
    for (const auto& result : data.comparisonResults) {
        const SimulationConfig* planConfig = nullptr;
        for (const auto& plan : data.comparisonPlans) {
            if (plan.name == result.planName) {
                planConfig = &plan.config;
                break;
            }
        }

        table += "  Plan: " + result.planName + "\r\n";
        if (planConfig != nullptr) {
            AppendLevelConfigTxt(table, "L1", planConfig->l1);
            AppendLevelConfigTxt(table, "L2", planConfig->l2);
        } else {
            table += "  Plan configuration not available.\r\n";
        }

        const auto& statistics = result.statistics;
        table += "  Accesses " + std::to_string(statistics.accesses) +
            " | Reads " + std::to_string(statistics.reads) +
            " | Writes " + std::to_string(statistics.writes) +
            " | L1 hits " + std::to_string(statistics.l1Hits) +
            " | L2 hits " + std::to_string(statistics.l2Hits) +
            " | Memory misses " + std::to_string(statistics.memoryMisses) + "\r\n";
        table += "  L1 hit rate " + PercentText(statistics.L1HitRate()) +
            " | L2 hit rate " + PercentText(statistics.L2HitRate()) +
            " | Overall hit rate " + PercentText(statistics.OverallHitRate()) +
            " | Miss rate " + PercentText(statistics.MissRate()) + "\r\n";
        table += "\r\n";
    }
    return table;
}

// Objective conclusion only: report facts produced by the runs, never guess
// why a plan performed the way it did.
std::string ExperimentConclusionText(const ExperimentExportData& data) {
    if (data.accessResults.empty()) {
        return "No accesses were executed.";
    }

    std::ostringstream stream;
    stream << "Executed " << data.accessResults.size()
           << " accesses with an overall hit rate of "
           << PercentText(data.statistics.OverallHitRate())
           << " (L1 " << PercentText(data.statistics.L1HitRate())
           << ", L2 " << PercentText(data.statistics.L2HitRate())
           << ", memory miss " << PercentText(data.statistics.MissRate()) << ").";
    return stream.str();
}

std::string ComparisonConclusionText(const ExperimentExportData& data) {
    if (data.comparisonResults.empty()) {
        return "No comparison results were provided.";
    }

    double best = data.comparisonResults.front().statistics.OverallHitRate();
    for (const auto& result : data.comparisonResults) {
        best = (std::max)(best, result.statistics.OverallHitRate());
    }

    std::string names;
    for (const auto& result : data.comparisonResults) {
        if (result.statistics.OverallHitRate() == best) {
            if (!names.empty()) {
                names += ", ";
            }
            names += result.planName;
        }
    }

    std::ostringstream stream;
    stream << "Highest overall hit rate: " << names
           << " (" << PercentText(best) << ").";
    return stream.str();
}

std::string PathForMessage(const std::filesystem::path& path) {
    try {
        return path.u8string();
    } catch (...) {
        return "(unprintable path)";
    }
}

}  // namespace

std::string ExperimentExporter::FormatExperimentCsv(const ExperimentExportData& data) {
    std::string csv;
    csv += CsvRow({kExperimentTitle});
    csv += CsvRow({"Program", kProgramVersion});
    csv += CsvRow({"Exported at", data.exportTime});
    csv += "\r\n";

    csv += "[Configuration]\r\n";
    csv += ConfigurationCsvRows(data.config);
    csv += "\r\n";

    csv += "[Trace Summary]\r\n";
    csv += TraceSummaryCsvRows(data);
    csv += "\r\n";

    csv += "[Trace]\r\n";
    csv += TraceCsvRows(data.trace);
    csv += "\r\n";

    csv += "[Per-Access Results]\r\n";
    csv += CsvRow({
        "Index", "Type", "Address (hex)", "Address (decimal)", "Outcome",
        "L1 Hit", "L1 Set", "L1 Line", "L1 Evicted", "L1 Evicted Block",
        "L2 Accessed", "L2 Hit", "L2 Set", "L2 Line", "L2 Evicted", "L2 Evicted Block"});
    for (std::size_t index = 0; index < data.accessResults.size(); ++index) {
        const auto& result = data.accessResults[index];
        const bool l2Accessed = result.outcome != AccessOutcome::L1Hit;
        csv += CsvRow({
            std::to_string(index),
            AccessTypeName(result.request),
            HexAddressText(result.request.address),
            std::to_string(result.request.address),
            ToString(result.outcome),
            result.l1.hit ? "yes" : "no",
            std::to_string(result.l1.setIndex),
            LineIndexText(result.l1.lineIndex),
            result.l1.evicted ? "yes" : "no",
            result.l1.evicted ? std::to_string(result.l1.evictedBlock) : std::string{},
            l2Accessed ? "yes" : "no",
            l2Accessed ? (result.l2.hit ? "yes" : "no") : std::string{},
            l2Accessed ? std::to_string(result.l2.setIndex) : std::string{},
            l2Accessed ? LineIndexText(result.l2.lineIndex) : std::string{},
            l2Accessed ? (result.l2.evicted ? "yes" : "no") : std::string{},
            l2Accessed && result.l2.evicted ? std::to_string(result.l2.evictedBlock) : std::string{}});
    }
    csv += "\r\n";

    csv += "[Final Statistics]\r\n";
    csv += FinalStatisticsCsvRows(data.statistics);
    csv += "\r\n";

    if (!data.comparisonResults.empty()) {
        csv += "[Comparison]\r\n";
        csv += ComparisonCsvTable(data);
        csv += "\r\n";
    }

    csv += "[Conclusion]\r\n";
    csv += CsvRow({"Conclusion", ExperimentConclusionText(data)});
    if (!data.comparisonResults.empty()) {
        csv += CsvRow({"Conclusion", ComparisonConclusionText(data)});
    }
    return csv;
}

std::string ExperimentExporter::FormatExperimentTxt(const ExperimentExportData& data) {
    std::string text;
    const std::string separator(64, '=');
    text += separator + "\r\n";
    text += std::string(kExperimentTitle) + "\r\n";
    text += separator + "\r\n";
    text += std::string("Program     : ") + kProgramVersion + "\r\n";
    text += "Exported at : " + data.exportTime + "\r\n";
    text += "\r\n";

    AppendLevelConfigTxt(text, "L1", data.config.l1);
    text += "\r\n";
    AppendLevelConfigTxt(text, "L2", data.config.l2);
    text += "\r\n";

    AppendTraceSummaryTxt(text, data);
    text += "\r\n";

    AppendTraceTxt(text, data.trace);
    text += "\r\n";

    text += "[Per-Access Results]\r\n";
    text += "  Index  Type  Address          Outcome        L1 detail";
    text += "                                     L2 detail\r\n";
    for (std::size_t index = 0; index < data.accessResults.size(); ++index) {
        text += "  " + AccessResultTxtRow(index, data.accessResults[index]) + "\r\n";
    }
    if (data.accessResults.size() < data.trace.size()) {
        text += "  (Only the executed prefix of the trace is listed above.)\r\n";
    }
    text += "\r\n";

    AppendFinalStatisticsTxt(text, data.statistics);
    text += "\r\n";

    if (!data.comparisonResults.empty()) {
        text += "[Comparison]\r\n";
        text += ComparisonTxtTable(data);
    }

    text += "[Conclusion]\r\n";
    text += "  " + ExperimentConclusionText(data) + "\r\n";
    if (!data.comparisonResults.empty()) {
        text += "  " + ComparisonConclusionText(data) + "\r\n";
    }
    return text;
}

std::string ExperimentExporter::FormatComparisonCsv(const ExperimentExportData& data) {
    std::string csv;
    csv += CsvRow({kComparisonTitle});
    csv += CsvRow({"Program", kProgramVersion});
    csv += CsvRow({"Exported at", data.exportTime});
    csv += "\r\n";

    csv += "[Trace Summary]\r\n";
    csv += TraceSummaryCsvRows(data);
    csv += "\r\n";

    csv += "[Trace]\r\n";
    csv += TraceCsvRows(data.trace);
    csv += "\r\n";

    csv += "[Comparison]\r\n";
    csv += ComparisonCsvTable(data);
    csv += "\r\n";

    csv += "[Conclusion]\r\n";
    csv += CsvRow({"Conclusion", ComparisonConclusionText(data)});
    return csv;
}

std::string ExperimentExporter::FormatComparisonTxt(const ExperimentExportData& data) {
    std::string text;
    const std::string separator(64, '=');
    text += separator + "\r\n";
    text += std::string(kComparisonTitle) + "\r\n";
    text += separator + "\r\n";
    text += std::string("Program     : ") + kProgramVersion + "\r\n";
    text += "Exported at : " + data.exportTime + "\r\n";
    text += "\r\n";

    AppendTraceSummaryTxt(text, data);
    text += "\r\n";

    AppendTraceTxt(text, data.trace);
    text += "\r\n";

    text += "[Comparison]\r\n";
    text += ComparisonTxtTable(data);

    text += "[Conclusion]\r\n";
    text += "  " + ComparisonConclusionText(data) + "\r\n";
    return text;
}

void ExperimentExporter::WriteUtf8File(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        throw std::runtime_error(
            "Unable to open the export file for writing: " + PathForMessage(path));
    }

    static constexpr char kUtf8Bom[] = "\xEF\xBB\xBF";
    file.write(kUtf8Bom, 3);
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    file.close();
    if (!file) {
        throw std::runtime_error(
            "Failed while writing the export file (disk full or access denied): " +
            PathForMessage(path));
    }
}

std::string ExperimentExporter::CurrentTimeStamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &time);
#else
    localtime_r(&time, &local);
#endif
    std::ostringstream stream;
    stream << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

std::string ExperimentExporter::FileTimeStamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &time);
#else
    localtime_r(&time, &local);
#endif
    std::ostringstream stream;
    stream << std::put_time(&local, "%Y%m%d_%H%M%S");
    return stream.str();
}

}  // namespace b5cache
