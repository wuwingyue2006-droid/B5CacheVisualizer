#include "mapping/IMappingStrategy.h"

#include <stdexcept>

namespace b5cache {
namespace {

void RequirePositiveDimensions(
    const std::size_t lineCount,
    const std::size_t associativity) {
    if (lineCount == 0) {
        throw std::invalid_argument("Line count must be greater than zero.");
    }
    if (associativity == 0) {
        throw std::invalid_argument("Associativity must be greater than zero.");
    }
}

class DirectMappingStrategy final : public IMappingStrategy {
public:
    AddressMapping Locate(
        const std::uint64_t blockNumber,
        const std::size_t lineCount,
        const std::size_t associativity) const override {
        RequirePositiveDimensions(lineCount, associativity);
        if (associativity != 1) {
            throw std::invalid_argument("Direct mapping requires associativity equal to 1.");
        }

        return {
            static_cast<std::size_t>(blockNumber % lineCount),
            blockNumber / lineCount,
            lineCount};
    }

    MappingKind Kind() const noexcept override {
        return MappingKind::Direct;
    }
};

class FullyAssociativeMappingStrategy final : public IMappingStrategy {
public:
    AddressMapping Locate(
        const std::uint64_t blockNumber,
        const std::size_t lineCount,
        const std::size_t associativity) const override {
        RequirePositiveDimensions(lineCount, associativity);
        if (associativity != lineCount) {
            throw std::invalid_argument(
                "Fully associative mapping requires associativity equal to line count.");
        }

        return {0, blockNumber, 1};
    }

    MappingKind Kind() const noexcept override {
        return MappingKind::FullyAssociative;
    }
};

class SetAssociativeMappingStrategy final : public IMappingStrategy {
public:
    AddressMapping Locate(
        const std::uint64_t blockNumber,
        const std::size_t lineCount,
        const std::size_t associativity) const override {
        RequirePositiveDimensions(lineCount, associativity);
        if (associativity <= 1 || associativity >= lineCount) {
            throw std::invalid_argument(
                "Set associative mapping requires associativity greater than 1 and less than line count.");
        }
        if (lineCount % associativity != 0) {
            throw std::invalid_argument("Line count must be divisible by associativity.");
        }

        const std::size_t setCount = lineCount / associativity;
        return {
            static_cast<std::size_t>(blockNumber % setCount),
            blockNumber / setCount,
            setCount};
    }

    MappingKind Kind() const noexcept override {
        return MappingKind::SetAssociative;
    }
};

}  // namespace

std::unique_ptr<IMappingStrategy> CreateMappingStrategy(const MappingKind kind) {
    switch (kind) {
    case MappingKind::Direct:
        return std::make_unique<DirectMappingStrategy>();
    case MappingKind::FullyAssociative:
        return std::make_unique<FullyAssociativeMappingStrategy>();
    case MappingKind::SetAssociative:
        return std::make_unique<SetAssociativeMappingStrategy>();
    }
    throw std::invalid_argument("Unknown mapping strategy.");
}

}  // namespace b5cache
