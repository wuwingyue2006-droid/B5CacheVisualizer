# B5CacheVisualizer 公共接口约定

> 代码头文件是最终事实来源，本文档用于帮助成员和 AI 快速理解。任何签名变化必须同步修改代码、本文档和受影响测试。

## 1. 公共命名空间

所有核心公共类型和接口均位于：

```cpp
namespace b5cache {
}
```

`src/` 中不得直接暴露 MFC 类型。

## 2. 公共数据类型

公共类型定义在 `src/common/CacheTypes.h`。

### 2.1 MappingKind

```cpp
enum class MappingKind {
    Direct,
    FullyAssociative,
    SetAssociative
};
```

### 2.2 ReplacementKind

```cpp
enum class ReplacementKind {
    Fifo,
    Lru
};
```

### 2.3 CacheLevelConfig

```cpp
struct CacheLevelConfig {
    std::string name;
    std::size_t sizeBytes;
    std::size_t blockSizeBytes;
    std::size_t associativity;
    MappingKind mapping;
    ReplacementKind replacement;
};
```

固定含义：

- `sizeBytes`：该级 Cache 总字节数；
- `blockSizeBytes`：每个 Cache Block 的字节数；
- `lineCount = sizeBytes / blockSizeBytes`；
- `associativity`：每组的 line 数；
- `setCount = lineCount / associativity`。

有效配置约束：

- Size 和 Block Size 必须大于 0；
- Size 必须能被 Block Size 整除；
- associativity 必须能整除 lineCount；
- Direct Mapping 的 associativity 必须为 1；
- Fully Associative 的 associativity 必须等于 lineCount；
- Set Associative 必须满足 `1 < associativity < lineCount`。

### 2.4 MemoryAccess

```cpp
struct MemoryAccess {
    std::uint64_t address;
    bool isWrite;
};
```

`isWrite == false` 表示读，`true` 表示写。

### 2.5 AddressMapping

```cpp
struct AddressMapping {
    std::size_t setIndex;
    std::uint64_t tag;
    std::size_t setCount;
};
```

成员 B 负责保证三种映射都返回这一统一结构。

### 2.6 CacheLineState

```cpp
struct CacheLineState {
    bool valid;
    bool dirty;
    std::uint64_t tag;
    std::uint64_t blockNumber;
    std::uint64_t insertedAt;
    std::uint64_t lastUsedAt;
};
```

- `insertedAt` 供 FIFO 使用；
- `lastUsedAt` 供 LRU 使用；
- UI 只能读取这些字段，不能直接修改 Cache Line。

### 2.7 AccessResult

```cpp
struct AccessResult {
    MemoryAccess request;
    AccessOutcome outcome;
    LevelAccessDetail l1;
    LevelAccessDetail l2;
};
```

`AccessOutcome` 只能是：

- `L1Hit`：请求在 L1 命中；
- `L2Hit`：L1 未命中、L2 命中；
- `MemoryMiss`：L1 和 L2 均未命中，需要从内存填充。

`LevelAccessDetail` 提供该级的 hit、set、line 和被驱逐 block 信息。UI 应以此为本次高亮和动画依据。

### 2.8 StatisticsSnapshot

```cpp
struct StatisticsSnapshot {
    std::uint64_t accesses;
    std::uint64_t reads;
    std::uint64_t writes;
    std::uint64_t l1Hits;
    std::uint64_t l2Hits;
    std::uint64_t memoryMisses;

    double L1HitRate() const noexcept;
    double L2HitRate() const noexcept;
    double OverallHitRate() const noexcept;
    double MissRate() const noexcept;
};
```

分母为 0 时命中率必须返回 `0.0`，不能出现除零或 NaN。

## 3. 映射接口

文件：`src/mapping/IMappingStrategy.h`

```cpp
class IMappingStrategy {
public:
    virtual AddressMapping Locate(
        std::uint64_t blockNumber,
        std::size_t lineCount,
        std::size_t associativity) const = 0;

    virtual MappingKind Kind() const noexcept = 0;
};
```

规则：

- 不读取或修改 Cache Line；
- 不决定 victim；
- 不维护统计；
- 只完成 block number 到 set index、tag 的映射；
- 无效输入抛出 `std::invalid_argument`。

## 4. 替换接口

文件：`src/replacement/IReplacementPolicy.h`

```cpp
class IReplacementPolicy {
public:
    virtual std::size_t SelectVictim(
        const std::vector<CacheLineState>& lines) const = 0;

    virtual void OnHit(
        CacheLineState& line,
        std::uint64_t tick) const = 0;

    virtual void OnInsert(
        CacheLineState& line,
        std::uint64_t tick) const = 0;

    virtual ReplacementKind Kind() const noexcept = 0;
};
```

规则：

- `CacheLevel` 优先选择 invalid line，只有组内全部 valid 才调用 `SelectVictim`；
- `SelectVictim` 返回组内下标，不能返回全 Cache 下标；
- FIFO 根据 `insertedAt`；
- LRU 根据 `lastUsedAt`；
- 相同时间戳时选择较小下标，保证结果可复现；
- 空集合必须抛出 `std::invalid_argument`。

## 5. CacheLevel 接口

文件：`src/core/CacheLevel.h`

```cpp
LevelAccessDetail Probe(
    std::uint64_t blockNumber,
    bool isWrite,
    std::uint64_t tick);

LevelAccessDetail Insert(
    std::uint64_t blockNumber,
    bool isWrite,
    std::uint64_t tick);

void Reset();

const CacheLevelConfig& Config() const noexcept;
const std::vector<std::vector<CacheLineState>>& Sets() const noexcept;
std::size_t LineCount() const noexcept;
```

规则：

- `Probe` 只在命中时修改 dirty 和策略时间；
- `Insert` 负责选择空 line 或调用替换策略；
- `Sets()` 只读暴露，主要供 UI 和测试查看；
- UI 不得持有可写的 CacheLine 引用。

## 6. CacheSimulator 接口

文件：`src/core/CacheSimulator.h`

```cpp
static SimulationConfig DefaultConfig();

explicit CacheSimulator(
    SimulationConfig config = DefaultConfig());

AccessResult Access(const MemoryAccess& request);
std::vector<AccessResult> Run(
    const std::vector<MemoryAccess>& trace);
void Reset();

const SimulationConfig& Config() const noexcept;
const CacheLevel& L1() const noexcept;
const CacheLevel& L2() const noexcept;
StatisticsSnapshot Statistics() const noexcept;
```

UI 的标准调用方式：

```cpp
b5cache::CacheSimulator simulator(config);
const auto accesses = b5cache::MemoryTraceParser::ParseText(text);
const auto results = simulator.Run(accesses);
const auto statistics = simulator.Statistics();
const auto& l1Sets = simulator.L1().Sets();
const auto& l2Sets = simulator.L2().Sets();
```

需要应用新配置时，UI 创建新的 `CacheSimulator`，不要尝试直接修改 `Config()` 返回的只读对象。

## 7. Trace 接口

文件：`src/trace/MemoryTraceParser.h`

```cpp
static std::vector<MemoryAccess> ParseText(
    const std::string& text);

static std::vector<MemoryAccess> ParseFile(
    const std::filesystem::path& path);
```

当前支持：

```text
R 0x10
W 32
48
# comment
```

- `R/r` 表示读；
- `W/w` 表示写；
- 省略 R/W 时默认读；
- 支持十进制和 `0x` 十六进制；
- `#` 后面为注释；
- 无效行抛出包含行号的 `std::invalid_argument`。

`ParseFile` 的签名已固定，当前骨架保留 `TODO(E)`。成员 E 负责读取文件文本并复用 `ParseText`，不要复制另一套解析规则。

## 8. 公共接口修改流程

任何成员或 AI 想修改本文件中的结构或签名，必须先在群里提交：

```text
【公共接口修改申请】
提出成员：
修改原因：
原接口：
新接口：
影响模块：A/B/C/D/E
需要其他成员同步修改的文件：
兼容方案：
```

获得组长同意后：

1. 修改头文件；
2. 修改实现；
3. 修改本文档；
4. 更新所有相关测试；
5. 在 PR 中突出说明接口变化；
6. 通知所有受影响成员更新 `dev`。
