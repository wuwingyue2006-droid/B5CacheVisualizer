#include "trace/MemoryTraceParser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
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

std::uint64_t ParseAddress(const std::string& token, const std::size_t lineNumber) {
    if (token.empty() || token.front() == '-' || token.front() == '+') {
        throw std::invalid_argument(
            "Trace line " + std::to_string(lineNumber) + " has an invalid address: " + token);
    }

    std::string digits = token;
    auto base = 10;
    if (token.size() >= 2 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X')) {
        digits = token.substr(2);
        base = 16;
    }

    try {
        std::size_t consumed = 0;
        const auto address = std::stoull(digits, &consumed, base);
        if (digits.empty() || consumed != digits.size()) {
            throw std::invalid_argument("trailing characters");
        }
        return address;
    } catch (const std::exception&) {
        throw std::invalid_argument(
            "Trace line " + std::to_string(lineNumber) + " has an invalid address: " + token);
    }
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

        std::string extraToken;
        if (lineInput >> extraToken) {
            throw std::invalid_argument(
                "Trace line " + std::to_string(lineNumber) + " has an unexpected token: " + extraToken);
        }

        accesses.push_back({ParseAddress(addressToken, lineNumber), isWrite});
    }

    return accesses;
}

std::vector<MemoryAccess> MemoryTraceParser::ParseFile(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to open trace file: " + path.string());
    }

    std::ostringstream text;
    text << input.rdbuf();
    if (input.bad()) {
        throw std::runtime_error("Unable to read trace file: " + path.string());
    }

    return ParseText(text.str());
}

}  // namespace b5cache
