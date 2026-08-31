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

        // FIFO: 选择 insertedAt 最小的行（最早进入）
        // 时间相同时选较小下标，保证结果可复现
        std::size_t victim = 0;
        for (std::size_t index = 1; index < lines.size(); ++index) {
            if (lines[index].insertedAt < lines[victim].insertedAt) {
                victim = index;
            }
        }
        return victim;
    }

    void OnHit(CacheLineState& line, const std::uint64_t tick) const override {
        // FIFO Hit 不影响 insertedAt，因此不会改变淘汰顺序
        // lastUsedAt 仍可更新用于统计/调试，但不参与 FIFO 决策
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
    std::size_t SelectVictim(const std::vector<CacheLineState>& lines) const override {
        if (lines.empty()) {
            throw std::invalid_argument("Cannot select a victim from an empty cache set.");
        }

        // LRU: 选择 lastUsedAt 最小的行（最久未使用）
        // 时间相同时选较小下标，保证结果可复现
        std::size_t victim = 0;
        for (std::size_t index = 1; index < lines.size(); ++index) {
            if (lines[index].lastUsedAt < lines[victim].lastUsedAt) {
                victim = index;
            }
        }
        return victim;
    }

    void OnHit(CacheLineState& line, const std::uint64_t tick) const override {
        // LRU Hit 必须更新最近使用时间，影响后续 victim 选择
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
