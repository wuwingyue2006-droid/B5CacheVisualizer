#include "TestSuites.h"

#include "mapping/IMappingStrategy.h"

namespace b5cache::tests {
namespace {

void TestDirectAddressLocation() {
    const auto mapping = CreateMappingStrategy(MappingKind::Direct);
    const auto location = mapping->Locate(9, 8, 1);

    Require(location.setIndex == 1, "Direct mapping set index should be block modulo line count.");
    Require(location.tag == 1, "Direct mapping tag should be block divided by line count.");
    Require(location.setCount == 8, "Direct mapping set count should equal line count.");
}

}  // namespace

void AddMappingTests(TestList& tests) {
    tests.push_back({"Mapping: direct address location", TestDirectAddressLocation});
    // TODO(B): add fully-associative and set-associative cases here.
}

}  // namespace b5cache::tests
