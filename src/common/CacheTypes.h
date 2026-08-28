#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace b5cache {

enum class MappingKind {
    Direct,
    FullyAssociative,
    SetAssociative
};

enum class ReplacementKind {
    Fifo,
    Lru
};

enum class AccessOutcome {
    L1Hit,
    L2Hit,
    MemoryMiss
};

struct CacheLevelConfig {
    std::string name;
    std::size_t sizeBytes = 0;
    std::size_t blockSizeBytes = 0;
    std::size_t associativity = 0;
    MappingKind mapping = MappingKind::Direct;
    ReplacementKind replacement = ReplacementKind::Fifo;
};

struct SimulationConfig {
    CacheLevelConfig l1;
    CacheLevelConfig l2;
};

struct MemoryAccess {
    std::uint64_t address = 0;
    bool isWrite = false;
};

struct AddressMapping {
    std::size_t setIndex = 0;
    std::uint64_t tag = 0;
    std::size_t setCount = 0;
};

struct CacheLineState {
    bool valid = false;
    bool dirty = false;
    std::uint64_t tag = 0;
    std::uint64_t blockNumber = 0;
    std::uint64_t insertedAt = 0;
    std::uint64_t lastUsedAt = 0;
};

inline constexpr std::size_t kInvalidIndex = static_cast<std::size_t>(-1);

struct LevelAccessDetail {
    bool hit = false;
    std::size_t setIndex = 0;
    std::size_t lineIndex = kInvalidIndex;
    bool evicted = false;
    std::uint64_t evictedBlock = 0;
};

struct AccessResult {
    MemoryAccess request;
    AccessOutcome outcome = AccessOutcome::MemoryMiss;
    LevelAccessDetail l1;
    LevelAccessDetail l2;
};

struct StatisticsSnapshot {
    std::uint64_t accesses = 0;
    std::uint64_t reads = 0;
    std::uint64_t writes = 0;
    std::uint64_t l1Hits = 0;
    std::uint64_t l2Hits = 0;
    std::uint64_t memoryMisses = 0;

    double L1HitRate() const noexcept;
    double L2HitRate() const noexcept;
    double OverallHitRate() const noexcept;
    double MissRate() const noexcept;
};

const char* ToString(MappingKind value) noexcept;
const char* ToString(ReplacementKind value) noexcept;
const char* ToString(AccessOutcome value) noexcept;

}  // namespace b5cache
