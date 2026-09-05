#include "ComparisonRunner.h"

#include "core/CacheSimulator.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>

namespace b5cache {
namespace {

std::string TrimCopy(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](const unsigned char character) {
        return std::isspace(character) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](const unsigned char character) {
        return std::isspace(character) != 0;
    }).base();
    return first >= last ? std::string{} : std::string(first, last);
}

}  // namespace

std::vector<ComparisonResult> ComparisonRunner::Run(
    const std::vector<ComparisonPlan>& plans,
    const std::vector<MemoryAccess>& trace) {
    if (plans.size() < 2 || plans.size() > 3) {
        throw std::invalid_argument("Comparison requires two or three plans.");
    }

    std::unordered_set<std::string> names;
    std::vector<ComparisonResult> results;
    results.reserve(plans.size());
    for (const auto& plan : plans) {
        const auto name = TrimCopy(plan.name);
        if (name.empty()) {
            throw std::invalid_argument("Every comparison plan needs a name.");
        }
        if (!names.insert(name).second) {
            throw std::invalid_argument("Comparison plan names must be distinct: " + name);
        }

        try {
            CacheSimulator simulator(plan.config);
            static_cast<void>(simulator.Run(trace));
            results.push_back({name, simulator.Statistics()});
        } catch (const std::exception& error) {
            throw std::invalid_argument("Plan '" + name + "' is invalid: " + error.what());
        }
    }
    return results;
}

}  // namespace b5cache
