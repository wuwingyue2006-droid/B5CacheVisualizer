#include "TestSuites.h"

#include "replacement/IReplacementPolicy.h"

namespace b5cache::tests {
namespace {

void TestFifoVictim() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    const std::vector<CacheLineState> lines = {
        {true, false, 0, 0, 8, 20},
        {true, false, 1, 1, 3, 30},
        {true, false, 2, 2, 5, 10},
    };

    Require(policy->SelectVictim(lines) == 1, "FIFO should select the earliest inserted line.");
}

}  // namespace

void AddReplacementTests(TestList& tests) {
    tests.push_back({"Replacement: FIFO victim", TestFifoVictim});
    // TODO(C): add LRU ordering, hit update and tie-breaking cases here.
}

}  // namespace b5cache::tests
