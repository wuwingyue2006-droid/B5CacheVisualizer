#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace b5cache::tests {

using TestCase = std::pair<std::string, std::function<void()>>;
using TestList = std::vector<TestCase>;

inline void Require(const bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

}  // namespace b5cache::tests
