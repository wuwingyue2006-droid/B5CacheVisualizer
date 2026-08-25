#include "trace/MemoryTraceParser.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace b5cache {
namespace {

std::string Trim(std::string value) {
    const auto isNotSpace = [](const unsigned char character) { return !std::isspace(character); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), isNotSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), isNotSpace).base(), value.end());
    return value;
}

}  // namespace

std::vector<MemoryAccess> MemoryTraceParser::ParseText(const std::string& text) {
    std::vector<MemoryAccess> accesses;
    std::istringstream input(text);
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(input, line)) {
        ++lineNumber;
        const auto comment = line.find('#');
        if (comment != std::string::npos) {
            line.erase(comment);
        }
        line = Trim(line);
        if (line.empty()) {
            continue;
        }

        std::istringstream lineInput(line);
        std::string firstToken;
        std::string addressToken;
        lineInput >> firstToken;

        bool isWrite = false;
        if (firstToken == "R" || firstToken == "r" || firstToken == "W" || firstToken == "w") {
            isWrite = firstToken == "W" || firstToken == "w";
            if (!(lineInput >> addressToken)) {
                throw std::invalid_argument("Trace line " + std::to_string(lineNumber) + " has no address.");
            }
        } else {
            addressToken = firstToken;
        }

        try {
            std::size_t consumed = 0;
            const auto address = std::stoull(addressToken, &consumed, 0);
            if (consumed != addressToken.size()) {
                throw std::invalid_argument("trailing characters");
            }
            accesses.push_back({address, isWrite});
        } catch (const std::exception&) {
            throw std::invalid_argument(
                "Trace line " + std::to_string(lineNumber) + " has an invalid address: " + addressToken);
        }
    }

    return accesses;
}

std::vector<MemoryAccess> MemoryTraceParser::ParseFile(const std::filesystem::path&) {
    // TODO(E): read the file and reuse ParseText so text and file imports share one format.
    throw std::logic_error("Trace file import is assigned to member E.");
}

}  // namespace b5cache
