#include "core/CacheLevel.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace b5cache {

CacheLevel::CacheLevel(CacheLevelConfig config)
    : config_(std::move(config)),
      lineCount_(ValidateAndGetLineCount(config_)),
      setCount_(GetSetCount(config_, lineCount_)),
      mapping_(CreateMappingStrategy(config_.mapping)),
      replacement_(CreateReplacementPolicy(config_.replacement)) {
    Reset();
}

LevelAccessDetail CacheLevel::Probe(
    const std::uint64_t blockNumber,
    const bool isWrite,
    const std::uint64_t tick) {
    const auto location = mapping_->Locate(blockNumber, lineCount_, config_.associativity);
    auto& set = sets_.at(location.setIndex);

    for (std::size_t index = 0; index < set.size(); ++index) {
        auto& line = set[index];
        if (line.valid && line.tag == location.tag) {
            line.dirty = line.dirty || isWrite;
            replacement_->OnHit(line, tick);
            return {true, location.setIndex, index, false, 0};
        }
    }

    return {false, location.setIndex, kInvalidIndex, false, 0};
}

LevelAccessDetail CacheLevel::Insert(
    const std::uint64_t blockNumber,
    const bool isWrite,
    const std::uint64_t tick) {
    const auto location = mapping_->Locate(blockNumber, lineCount_, config_.associativity);
    auto& set = sets_.at(location.setIndex);

    auto selected = std::find_if(set.begin(), set.end(), [](const CacheLineState& line) {
        return !line.valid;
    });

    std::size_t lineIndex = 0;
    bool evicted = false;
    std::uint64_t evictedBlock = 0;

    if (selected == set.end()) {
        lineIndex = replacement_->SelectVictim(set);
        selected = set.begin() + static_cast<std::ptrdiff_t>(lineIndex);
        evicted = selected->valid;
        evictedBlock = selected->blockNumber;
    } else {
        lineIndex = static_cast<std::size_t>(std::distance(set.begin(), selected));
    }

    *selected = {};
    selected->valid = true;
    selected->dirty = isWrite;
    selected->tag = location.tag;
    selected->blockNumber = blockNumber;
    replacement_->OnInsert(*selected, tick);

    return {false, location.setIndex, lineIndex, evicted, evictedBlock};
}

void CacheLevel::Reset() {
    sets_.assign(setCount_, std::vector<CacheLineState>(config_.associativity));
}

const CacheLevelConfig& CacheLevel::Config() const noexcept {
    return config_;
}

const std::vector<std::vector<CacheLineState>>& CacheLevel::Sets() const noexcept {
    return sets_;
}

std::size_t CacheLevel::LineCount() const noexcept {
    return lineCount_;
}

std::size_t CacheLevel::ValidateAndGetLineCount(const CacheLevelConfig& config) {
    if (config.sizeBytes == 0 || config.blockSizeBytes == 0) {
        throw std::invalid_argument(config.name + " size and block size must be positive.");
    }
    if (config.sizeBytes % config.blockSizeBytes != 0) {
        throw std::invalid_argument(config.name + " size must be divisible by block size.");
    }

    const auto lineCount = config.sizeBytes / config.blockSizeBytes;
    if (config.associativity == 0 || lineCount % config.associativity != 0) {
        throw std::invalid_argument(config.name + " associativity must divide the line count.");
    }

    switch (config.mapping) {
    case MappingKind::Direct:
        if (config.associativity != 1) {
            throw std::invalid_argument(config.name + " direct mapping requires associativity 1.");
        }
        break;
    case MappingKind::FullyAssociative:
        if (config.associativity != lineCount) {
            throw std::invalid_argument(config.name + " fully associative mapping requires one set.");
        }
        break;
    case MappingKind::SetAssociative:
        if (config.associativity <= 1 || config.associativity >= lineCount) {
            throw std::invalid_argument(config.name + " set associative mapping requires 1 < ways < line count.");
        }
        break;
    }

    return lineCount;
}

std::size_t CacheLevel::GetSetCount(const CacheLevelConfig& config, const std::size_t lineCount) {
    return lineCount / config.associativity;
}

}  // namespace b5cache
