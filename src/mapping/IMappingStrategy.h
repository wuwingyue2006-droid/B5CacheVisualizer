#pragma once

#include "common/CacheTypes.h"

#include <memory>

namespace b5cache {

class IMappingStrategy {
public:
    virtual ~IMappingStrategy() = default;

    virtual AddressMapping Locate(
        std::uint64_t blockNumber,
        std::size_t lineCount,
        std::size_t associativity) const = 0;

    virtual MappingKind Kind() const noexcept = 0;
};

std::unique_ptr<IMappingStrategy> CreateMappingStrategy(MappingKind kind);

}  // namespace b5cache
