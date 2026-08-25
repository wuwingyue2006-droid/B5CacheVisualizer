#include "mapping/IMappingStrategy.h"

#include <stdexcept>

namespace b5cache {
namespace {

class DirectMappingStrategy final : public IMappingStrategy {
public:
    AddressMapping Locate(
        const std::uint64_t blockNumber,
        const std::size_t lineCount,
        const std::size_t associativity) const override {
        if (lineCount == 0 || associativity != 1) {
            throw std::invalid_argument("Direct mapping requires non-zero lines and associativity 1.");
        }
        return {static_cast<std::size_t>(blockNumber % lineCount), blockNumber / lineCount, lineCount};
    }

    MappingKind Kind() const noexcept override {
        return MappingKind::Direct;
    }
};

class FullyAssociativeMappingStrategy final : public IMappingStrategy {
public:
    AddressMapping Locate(
        std::uint64_t,
        std::size_t,
        std::size_t) const override {
        // TODO(B): implement fully associative address mapping and its validation.
        throw std::logic_error("Fully associative mapping is assigned to member B.");
    }

    MappingKind Kind() const noexcept override {
        return MappingKind::FullyAssociative;
    }
};

class SetAssociativeMappingStrategy final : public IMappingStrategy {
public:
    AddressMapping Locate(
        std::uint64_t,
        std::size_t,
        std::size_t) const override {
        // TODO(B): implement set-associative address mapping and its validation.
        throw std::logic_error("Set associative mapping is assigned to member B.");
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
