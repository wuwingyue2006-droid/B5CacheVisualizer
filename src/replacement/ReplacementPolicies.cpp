#include "replacement/IReplacementPolicy.h"

#include <stdexcept>

namespace b5cache {
namespace {

class FifoReplacementPolicy final : public IReplacementPolicy {
public:
    std::size_t SelectVictim(const std::vector<CacheLineState>& lines) const override {
        if (lines.empty()) {
            throw std::invalid_argument("Cannot select a victim from an empty cache set.");
        }

        std::size_t victim = 0;
        for (std::size_t index = 1; index < lines.size(); ++index) {
            if (lines[index].insertedAt < lines[victim].insertedAt) {
                victim = index;
            }
        }
        return victim;
    }

    void OnHit(CacheLineState& line, const std::uint64_t tick) const override {
        line.lastUsedAt = tick;
    }

    void OnInsert(CacheLineState& line, const std::uint64_t tick) const override {
        line.insertedAt = tick;
        line.lastUsedAt = tick;
    }

    ReplacementKind Kind() const noexcept override {
        return ReplacementKind::Fifo;
    }
};

class LruReplacementPolicy final : public IReplacementPolicy {
public:
    std::size_t SelectVictim(const std::vector<CacheLineState>&) const override {
        // TODO(C): choose the least-recently-used valid line.
        throw std::logic_error("LRU replacement is assigned to member C.");
    }

    void OnHit(CacheLineState& line, const std::uint64_t tick) const override {
        // The timestamp update is part of the fixed interface; member C completes victim selection and tests.
        line.lastUsedAt = tick;
    }

    void OnInsert(CacheLineState& line, const std::uint64_t tick) const override {
        line.insertedAt = tick;
        line.lastUsedAt = tick;
    }

    ReplacementKind Kind() const noexcept override {
        return ReplacementKind::Lru;
    }
};

}  // namespace

std::unique_ptr<IReplacementPolicy> CreateReplacementPolicy(const ReplacementKind kind) {
    switch (kind) {
    case ReplacementKind::Fifo:
        return std::make_unique<FifoReplacementPolicy>();
    case ReplacementKind::Lru:
        return std::make_unique<LruReplacementPolicy>();
    }
    throw std::invalid_argument("Unknown replacement policy.");
}

}  // namespace b5cache
