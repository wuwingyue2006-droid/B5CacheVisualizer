#include "TestSuites.h"

#include "trace/MemoryTraceParser.h"

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

}  // namespace

void AddTraceTests(TestList& tests) {
    tests.push_back({"Trace: text parser", TestTraceParser});
    // TODO(E): add invalid-line, comments, empty input and file-import cases here.
}

}  // namespace b5cache::tests
