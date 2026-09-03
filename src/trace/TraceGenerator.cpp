#include "TraceGenerator.h"

#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>

namespace b5cache {
namespace {

constexpr std::size_t kMaximumRequestCount = 10000;
constexpr std::uint64_t kProbabilityScale = 1000000;

void Require(const bool condition, const char* message) {
    if (!condition) {
        throw std::invalid_argument(message);
    }
}

std::uint64_t CheckedAdd(const std::uint64_t left, const std::uint64_t right) {
    if (right > std::numeric_limits<std::uint64_t>::max() - left) {
        throw std::invalid_argument("Generated address overflows uint64_t.");
    }
    return left + right;
}

std::uint64_t CheckedMultiply(const std::uint64_t left, const std::uint64_t right) {
    if (left != 0 && right > std::numeric_limits<std::uint64_t>::max() / left) {
        throw std::invalid_argument("Generated address overflows uint64_t.");
    }
    return left * right;
}

std::size_t SlotCount(const TraceGenerationConfig& config) {
    Require(config.addressRangeBytes > 0, "Address range must be greater than zero.");
    Require(config.stepBytes > 0, "Step must be greater than zero.");
    static_cast<void>(CheckedAdd(config.startAddress, config.addressRangeBytes - 1));

    const auto slots = (config.addressRangeBytes - 1) / config.stepBytes + 1;
    Require(slots <= static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()),
            "Address range produces too many candidate addresses.");
    return static_cast<std::size_t>(slots);
}

std::uint64_t AddressAt(const TraceGenerationConfig& config, const std::size_t slot) {
    const auto offset = CheckedMultiply(static_cast<std::uint64_t>(slot), config.stepBytes);
    Require(offset < config.addressRangeBytes, "Step and address range do not form a valid address sequence.");
    return CheckedAdd(config.startAddress, offset);
}

std::size_t UniformIndex(std::mt19937_64& engine, const std::size_t upperExclusive) {
    Require(upperExclusive > 0, "Random address selection requires a non-empty range.");
    const auto bound = static_cast<std::uint64_t>(upperExclusive);
    const auto threshold = (std::uint64_t{0} - bound) % bound;
    std::uint64_t value = 0;
    do {
        value = engine();
    } while (value < threshold);
    return static_cast<std::size_t>(value % bound);
}

bool ChooseProbability(std::mt19937_64& engine, const double probability) {
    if (probability <= 0.0) {
        return false;
    }
    if (probability >= 1.0) {
        return true;
    }
    const auto threshold = static_cast<std::uint64_t>(std::llround(probability * kProbabilityScale));
    return UniformIndex(engine, static_cast<std::size_t>(kProbabilityScale)) < threshold;
}

void Validate(const TraceGenerationConfig& config) {
    Require(config.requestCount > 0, "Request count must be greater than zero.");
    Require(config.requestCount <= kMaximumRequestCount,
            "Request count must not exceed 10000 to keep the UI responsive.");
    Require(config.hotProbability >= 0.0 && config.hotProbability <= 1.0,
            "Hot-set probability must be between 0 and 1.");
    Require(config.writeProbability >= 0.0 && config.writeProbability <= 1.0,
            "Write probability must be between 0 and 1.");
    static_cast<void>(SlotCount(config));
}

}  // namespace

std::vector<MemoryAccess> TraceGenerator::Generate(const TraceGenerationConfig& config) {
    Validate(config);
    const auto slots = SlotCount(config);
    std::vector<MemoryAccess> accesses;
    accesses.reserve(config.requestCount);
    std::mt19937_64 engine(config.randomSeed);

    if (config.mode == TraceGenerationMode::Loop) {
        Require(config.loopLength > 0, "Loop length must be greater than zero.");
        Require(config.loopLength <= slots, "Loop length must fit inside the address range.");
    }
    if (config.mode == TraceGenerationMode::HotSet) {
        Require(config.hotSetSize > 0, "Hot-set size must be greater than zero.");
        Require(config.hotSetSize <= slots, "Hot-set size must fit inside the address range.");
    }

    for (std::size_t index = 0; index < config.requestCount; ++index) {
        std::size_t slot = 0;
        bool isWrite = false;
        switch (config.mode) {
        case TraceGenerationMode::Sequential:
            Require(index < slots, "Sequential request count does not fit inside the address range.");
            slot = index;
            break;
        case TraceGenerationMode::Loop:
            slot = index % config.loopLength;
            break;
        case TraceGenerationMode::Random:
            slot = UniformIndex(engine, slots);
            break;
        case TraceGenerationMode::HotSet:
            if (ChooseProbability(engine, config.hotProbability) || config.hotSetSize == slots) {
                slot = UniformIndex(engine, config.hotSetSize);
            } else {
                slot = config.hotSetSize + UniformIndex(engine, slots - config.hotSetSize);
            }
            break;
        case TraceGenerationMode::MixedReadWrite:
            slot = index % slots;
            isWrite = ChooseProbability(engine, config.writeProbability);
            break;
        default:
            throw std::invalid_argument("Unknown trace generation mode.");
        }
        accesses.push_back({AddressAt(config, slot), isWrite});
    }
    return accesses;
}

std::string TraceGenerator::FormatText(const std::vector<MemoryAccess>& accesses) {
    std::ostringstream text;
    text << std::uppercase << std::hex;
    for (const auto& access : accesses) {
        text << (access.isWrite ? 'W' : 'R') << " 0x" << access.address << '\n';
    }
    return text.str();
}

}  // namespace b5cache
