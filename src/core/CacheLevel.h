#pragma once

#include "common/CacheTypes.h"
#include "mapping/IMappingStrategy.h"
#include "replacement/IReplacementPolicy.h"

#include <memory>
#include <vector>

namespace b5cache {

class CacheLevel {
public:
    explicit CacheLevel(CacheLevelConfig config);

    LevelAccessDetail Probe(std::uint64_t blockNumber, bool isWrite, std::uint64_t tick);
    LevelAccessDetail Insert(std::uint64_t blockNumber, bool isWrite, std::uint64_t tick);
    void Reset();

    const CacheLevelConfig& Config() const noexcept;
    const std::vector<std::vector<CacheLineState>>& Sets() const noexcept;
    std::size_t LineCount() const noexcept;

private:
    static std::size_t ValidateAndGetLineCount(const CacheLevelConfig& config);
    static std::size_t GetSetCount(const CacheLevelConfig& config, std::size_t lineCount);

    CacheLevelConfig config_;
    std::size_t lineCount_ = 0;
    std::size_t setCount_ = 0;
    std::unique_ptr<IMappingStrategy> mapping_;
    std::unique_ptr<IReplacementPolicy> replacement_;
    std::vector<std::vector<CacheLineState>> sets_;
};

}  // namespace b5cache
