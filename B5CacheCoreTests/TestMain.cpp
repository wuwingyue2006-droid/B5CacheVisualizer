#include "TestSuites.h"

#include <exception>
#include <iostream>

int main() {
    b5cache::tests::TestList tests;
    b5cache::tests::AddCoreTests(tests);
    b5cache::tests::AddMappingTests(tests);
    b5cache::tests::AddReplacementTests(tests);
    b5cache::tests::AddStatisticsTests(tests);
    b5cache::tests::AddTraceTests(tests);
    b5cache::tests::AddComparisonTests(tests);

    std::size_t passed = 0;
    for (const auto& [name, test] : tests) {
        try {
            test();
            ++passed;
            std::cout << "[PASS] " << name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }

    std::cout << passed << "/" << tests.size() << " tests passed.\n";
    return passed == tests.size() ? 0 : 1;
}
