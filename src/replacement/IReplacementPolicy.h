#pragma once

#include "common/CacheTypes.h"

#include <memory>
#include <vector>

namespace b5cache {

class IReplacementPolicy {
public:
    virtual ~IReplacementPolicy() = default;

    virtual std::size_t SelectVictim(const std::vector<CacheLineState>& lines) const = 0;
    virtual void OnHit(CacheLineState& line, std::uint64_t tick) const = 0;
    virtual void OnInsert(CacheLineState& line, std::uint64_t tick) const = 0;
    virtual ReplacementKind Kind() const noexcept = 0;
};

std::unique_ptr<IReplacementPolicy> CreateReplacementPolicy(ReplacementKind kind);

}  // namespace b5cache
