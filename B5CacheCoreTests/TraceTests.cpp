#include "TestSuites.h"

#include "trace/MemoryTraceParser.h"
#include "trace/TraceGenerator.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace b5cache::tests {
namespace {

void TestTraceParser() {
    const auto trace = MemoryTraceParser::ParseText(
        "# example\n"
        "R 0x10\n"
        "W 32\n"
        "48\n");

    Require(trace.size() == 3, "Trace parser should return three accesses.");
    Require(trace[0].address == 0x10 && !trace[0].isWrite, "Hex read was parsed incorrectly.");
    Require(trace[1].address == 32 && trace[1].isWrite, "Decimal write was parsed incorrectly.");
    Require(trace[2].address == 48 && !trace[2].isWrite, "Implicit read was parsed incorrectly.");
}

void TestTraceWhitespaceCommentsAndCase() {
    const auto trace = MemoryTraceParser::ParseText(
        "  r 16  \n"
        "\n"
        "# only a comment\n"
        "w 0X20 # inline comment\n"
        "  08\n");

    Require(trace.size() == 3, "Whitespace and comments should not create accesses.");
    Require(trace[0].address == 16 && !trace[0].isWrite, "Lowercase read was parsed incorrectly.");
    Require(trace[1].address == 0x20 && trace[1].isWrite, "Uppercase hexadecimal write was parsed incorrectly.");
    Require(trace[2].address == 8 && !trace[2].isWrite, "Leading-zero decimal address was parsed incorrectly.");

    const auto emptyTrace = MemoryTraceParser::ParseText("\n# no accesses\n");
    Require(emptyTrace.empty(), "Empty and comment-only trace input should produce no accesses.");

    const auto largestAddress = MemoryTraceParser::ParseText("0xFFFFFFFFFFFFFFFF\n");
    Require(largestAddress.size() == 1 && largestAddress[0].address == UINT64_MAX,
            "The parser should support the largest uint64_t address.");
}

void RequireInvalidTrace(const std::string& text, const std::string& expectedMessage) {
    try {
        static_cast<void>(MemoryTraceParser::ParseText(text));
    } catch (const std::invalid_argument& error) {
        Require(std::string(error.what()).find(expectedMessage) != std::string::npos,
                "Trace parse error did not contain the expected detail.");
        return;
    }
    throw std::runtime_error("Invalid trace input should throw std::invalid_argument.");
}

void TestTraceInvalidInput() {
    RequireInvalidTrace("R\n", "line 1");
    RequireInvalidTrace("R 12 extra\n", "unexpected token: extra");
    RequireInvalidTrace("# comment\nR 0xZZ\n", "line 2");
    RequireInvalidTrace("0x-1\n", "line 1");
    RequireInvalidTrace("0x+1\n", "line 1");
    RequireInvalidTrace("18446744073709551616\n", "line 1");
}

void TestTraceFileImport() {
    const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto path = std::filesystem::temp_directory_path() /
        ("B5CacheVisualizer_trace_input_" + suffix + ".txt");
    {
        std::ofstream output(path, std::ios::trunc);
        Require(static_cast<bool>(output), "Test trace file could not be created.");
        output << "R 0x10\nW 32\n";
    }

    const auto trace = MemoryTraceParser::ParseFile(path);
    std::filesystem::remove(path);
    Require(trace.size() == 2, "Trace file import should return two accesses.");
    Require(trace[0].address == 0x10 && !trace[0].isWrite, "Trace file read was parsed incorrectly.");
    Require(trace[1].address == 32 && trace[1].isWrite, "Trace file write was parsed incorrectly.");

    const auto missingPath = std::filesystem::temp_directory_path() /
        ("B5CacheVisualizer_missing_trace_input_" + suffix + ".txt");
    try {
        static_cast<void>(MemoryTraceParser::ParseFile(missingPath));
    } catch (const std::runtime_error& error) {
        Require(std::string(error.what()).find("Unable to open trace file") != std::string::npos,
                "Missing trace file should have a clear error message.");
        return;
    }
    throw std::runtime_error("Missing trace file should throw std::runtime_error.");
}

void TestGeneratedTraceFormatsAndParses() {
    TraceGenerationConfig config;
    config.mode = TraceGenerationMode::Sequential;
    config.requestCount = 4;
    config.startAddress = 0x20;
    config.addressRangeBytes = 64;
    config.stepBytes = 16;

    const auto generated = TraceGenerator::Generate(config);
    Require(generated.size() == 4, "Sequential generator should create the requested count.");
    Require(generated[0].address == 0x20 && generated[3].address == 0x50,
            "Sequential generator should advance by the requested step.");
    Require(MemoryTraceParser::ParseText(TraceGenerator::FormatText(generated)).size() == generated.size(),
            "Formatted generated trace should be accepted by the existing parser.");
}

void TestGeneratedLoopAndSeededModes() {
    TraceGenerationConfig loop;
    loop.mode = TraceGenerationMode::Loop;
    loop.requestCount = 6;
    loop.addressRangeBytes = 64;
    loop.stepBytes = 16;
    loop.loopLength = 2;
    const auto loopTrace = TraceGenerator::Generate(loop);
    Require(loopTrace[0].address == 0 && loopTrace[1].address == 16 &&
                loopTrace[2].address == 0 && loopTrace[3].address == 16,
            "Loop generator should repeat exactly the requested working set.");

    TraceGenerationConfig random;
    random.mode = TraceGenerationMode::Random;
    random.requestCount = 12;
    random.addressRangeBytes = 256;
    random.stepBytes = 16;
    random.randomSeed = 42;
    Require(TraceGenerator::FormatText(TraceGenerator::Generate(random)) ==
                TraceGenerator::FormatText(TraceGenerator::Generate(random)),
            "Random generation must be reproducible for the same explicit seed.");

    random.mode = TraceGenerationMode::MixedReadWrite;
    random.writeProbability = 1.0;
    const auto writes = TraceGenerator::Generate(random);
    for (const auto& access : writes) {
        Require(access.isWrite, "A 100% write ratio must generate only write accesses.");
    }
}

void TestGeneratedTraceValidation() {
    auto requireInvalid = [](const TraceGenerationConfig& config, const std::string& message) {
        try {
            static_cast<void>(TraceGenerator::Generate(config));
        } catch (const std::invalid_argument&) {
            return;
        }
        throw std::runtime_error(message);
    };

    TraceGenerationConfig invalid;
    invalid.requestCount = 0;
    requireInvalid(invalid, "A zero request count should be rejected.");

    invalid.requestCount = 2;
    invalid.startAddress = UINT64_MAX;
    invalid.addressRangeBytes = 2;
    requireInvalid(invalid, "Address-range overflow should be rejected.");
}

}  // namespace

void AddTraceTests(TestList& tests) {
    tests.push_back({"Trace: text parser", TestTraceParser});
    tests.push_back({"Trace: whitespace, comments and case", TestTraceWhitespaceCommentsAndCase});
    tests.push_back({"Trace: invalid input", TestTraceInvalidInput});
    tests.push_back({"Trace: file import", TestTraceFileImport});
    tests.push_back({"Trace generator: formatting and parser compatibility", TestGeneratedTraceFormatsAndParses});
    tests.push_back({"Trace generator: loop and seeded modes", TestGeneratedLoopAndSeededModes});
    tests.push_back({"Trace generator: validation", TestGeneratedTraceValidation});
}

}  // namespace b5cache::tests
