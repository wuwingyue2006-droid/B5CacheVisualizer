#include "TestSuites.h"

#include "mapping/IMappingStrategy.h"

#include <stdexcept>
#include <string>

namespace b5cache::tests {
namespace {

template <typename Action>
void RequireInvalidArgument(Action action, const std::string& message) {
    try {
        action();
    } catch (const std::invalid_argument&) {
        return;
    } catch (...) {
        throw std::runtime_error(message + " Expected std::invalid_argument.");
    }
    throw std::runtime_error(message + " No exception was thrown.");
}

void TestDirectAddressLocation() {
    const auto mapping = CreateMappingStrategy(MappingKind::Direct);
    const auto location = mapping->Locate(9, 8, 1);
    Require(location.setIndex == 1, "Direct set index should be block modulo line count.");
    Require(location.tag == 1, "Direct tag should be block divided by line count.");
    Require(location.setCount == 8, "Direct set count should equal line count.");
}

void TestDirectBlockZero() {
    const auto mapping = CreateMappingStrategy(MappingKind::Direct);
    const auto location = mapping->Locate(0, 8, 1);
    Require(location.setIndex == 0 && location.tag == 0 && location.setCount == 8,
            "Direct mapping should handle block zero.");
}

void TestDirectLargeBlock() {
    const auto mapping = CreateMappingStrategy(MappingKind::Direct);
    const auto location = mapping->Locate(100, 8, 1);
    Require(location.setIndex == 4 && location.tag == 12,
            "Direct mapping should handle a large block number.");
}

void TestDirectKind() {
    Require(CreateMappingStrategy(MappingKind::Direct)->Kind() == MappingKind::Direct,
            "Direct strategy should report its kind.");
}

void TestFullyAssociativeLocation() {
    const auto mapping = CreateMappingStrategy(MappingKind::FullyAssociative);
    const auto location = mapping->Locate(9, 8, 8);
    Require(location.setIndex == 0, "Fully associative set index should be zero.");
    Require(location.tag == 9, "Fully associative tag should equal block number.");
    Require(location.setCount == 1, "Fully associative set count should be one.");
}

void TestFullyAssociativeBlockZero() {
    const auto mapping = CreateMappingStrategy(MappingKind::FullyAssociative);
    const auto location = mapping->Locate(0, 8, 8);
    Require(location.setIndex == 0 && location.tag == 0 && location.setCount == 1,
            "Fully associative mapping should handle block zero.");
}

void TestFullyAssociativeLargeBlock() {
    const auto mapping = CreateMappingStrategy(MappingKind::FullyAssociative);
    const auto location = mapping->Locate(999983, 8, 8);
    Require(location.setIndex == 0 && location.tag == 999983,
            "Fully associative mapping should preserve a large block as the tag.");
}

void TestFullyAssociativeKind() {
    Require(CreateMappingStrategy(MappingKind::FullyAssociative)->Kind() ==
                MappingKind::FullyAssociative,
            "Fully associative strategy should report its kind.");
}

void TestSetAssociative2Way() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    const auto location = mapping->Locate(9, 8, 2);
    Require(location.setCount == 4, "Two-way mapping should have four sets.");
    Require(location.setIndex == 1, "Two-way mapping should locate block 9 in set 1.");
    Require(location.tag == 2, "Two-way mapping should calculate tag 2.");
}

void TestSetAssociative4Way() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    const auto location = mapping->Locate(15, 16, 4);
    Require(location.setCount == 4, "Four-way mapping should have four sets.");
    Require(location.setIndex == 3, "Four-way mapping should locate block 15 in set 3.");
    Require(location.tag == 3, "Four-way mapping should calculate tag 3.");
}

void TestSetAssociativeBlockZero() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    const auto location = mapping->Locate(0, 8, 2);
    Require(location.setCount == 4 && location.setIndex == 0 && location.tag == 0,
            "Set associative mapping should handle block zero.");
}

void TestSetAssociativeKind() {
    Require(CreateMappingStrategy(MappingKind::SetAssociative)->Kind() ==
                MappingKind::SetAssociative,
            "Set associative strategy should report its kind.");
}

void TestDirectZeroLineCount() {
    const auto mapping = CreateMappingStrategy(MappingKind::Direct);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 0, 1); },
                           "Direct mapping should reject zero lines.");
}

void TestDirectZeroAssociativity() {
    const auto mapping = CreateMappingStrategy(MappingKind::Direct);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 0); },
                           "Direct mapping should reject zero associativity.");
}

void TestDirectWrongAssociativity() {
    const auto mapping = CreateMappingStrategy(MappingKind::Direct);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 2); },
                           "Direct mapping should reject associativity other than one.");
}

void TestFullyZeroLineCount() {
    const auto mapping = CreateMappingStrategy(MappingKind::FullyAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 0, 8); },
                           "Fully associative mapping should reject zero lines.");
}

void TestFullyZeroAssociativity() {
    const auto mapping = CreateMappingStrategy(MappingKind::FullyAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 0); },
                           "Fully associative mapping should reject zero associativity.");
}

void TestFullyWrongAssociativity() {
    const auto mapping = CreateMappingStrategy(MappingKind::FullyAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 4); },
                           "Fully associative mapping should require all lines as ways.");
}

void TestSetZeroLineCount() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 0, 2); },
                           "Set associative mapping should reject zero lines.");
}

void TestSetZeroAssociativity() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 0); },
                           "Set associative mapping should reject zero associativity.");
}

void TestSetNonDivisible() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 3); },
                           "Set associative mapping should reject non-divisible ways.");
}

void TestSetAssociativityOne() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 1); },
                           "Set associative mapping should reject one way.");
}

void TestSetAssociativityEqualsLines() {
    const auto mapping = CreateMappingStrategy(MappingKind::SetAssociative);
    RequireInvalidArgument([&] { (void)mapping->Locate(9, 8, 8); },
                           "Set associative mapping should reject all lines as ways.");
}

void TestCreateUnknownMappingThrows() {
    RequireInvalidArgument(
        [] { (void)CreateMappingStrategy(static_cast<MappingKind>(99)); },
        "The mapping factory should reject an unknown kind.");
}

}  // namespace

void AddMappingTests(TestList& tests) {
    tests.push_back({"Mapping: direct address location", TestDirectAddressLocation});
    tests.push_back({"Mapping: direct block zero", TestDirectBlockZero});
    tests.push_back({"Mapping: direct large block", TestDirectLargeBlock});
    tests.push_back({"Mapping: direct Kind()", TestDirectKind});
    tests.push_back({"Mapping: fully associative location", TestFullyAssociativeLocation});
    tests.push_back({"Mapping: fully associative block zero", TestFullyAssociativeBlockZero});
    tests.push_back({"Mapping: fully associative large block", TestFullyAssociativeLargeBlock});
    tests.push_back({"Mapping: fully associative Kind()", TestFullyAssociativeKind});
    tests.push_back({"Mapping: 2-way set associative", TestSetAssociative2Way});
    tests.push_back({"Mapping: 4-way set associative", TestSetAssociative4Way});
    tests.push_back({"Mapping: set associative block zero", TestSetAssociativeBlockZero});
    tests.push_back({"Mapping: set associative Kind()", TestSetAssociativeKind});
    tests.push_back({"Mapping: direct zero line count throws", TestDirectZeroLineCount});
    tests.push_back({"Mapping: direct zero associativity throws", TestDirectZeroAssociativity});
    tests.push_back({"Mapping: direct wrong associativity throws", TestDirectWrongAssociativity});
    tests.push_back({"Mapping: fully zero line count throws", TestFullyZeroLineCount});
    tests.push_back({"Mapping: fully zero associativity throws", TestFullyZeroAssociativity});
    tests.push_back({"Mapping: fully wrong associativity throws", TestFullyWrongAssociativity});
    tests.push_back({"Mapping: set zero line count throws", TestSetZeroLineCount});
    tests.push_back({"Mapping: set zero associativity throws", TestSetZeroAssociativity});
    tests.push_back({"Mapping: set non-divisible throws", TestSetNonDivisible});
    tests.push_back({"Mapping: set associativity 1 throws", TestSetAssociativityOne});
    tests.push_back({"Mapping: set associativity equals lines throws", TestSetAssociativityEqualsLines});
    tests.push_back({"Mapping: unknown kind throws", TestCreateUnknownMappingThrows});
}

}  // namespace b5cache::tests
