# 成员 E 任务卡：Statistics 与 Memory Trace

## 身份信息

```text
成员代号：E
负责模块：命中统计、命中率、Trace 文本解析和文件导入
个人分支：feature-statistics
前置条件：已确认代号 E；从最新 dev 创建 feature-statistics；工作区状态已检查
```

## 允许主要修改的文件

```text
src/statistics/CacheStatistics.h
src/statistics/CacheStatistics.cpp
src/trace/MemoryTraceParser.h
src/trace/MemoryTraceParser.cpp
B5CacheCoreTests/StatisticsTests.cpp
B5CacheCoreTests/TraceTests.cpp
```

如果需要给 `StatisticsSnapshot` 继续新增字段或方法，必须先提交公共接口修改申请。`MissRate()` 和 `ParseFile()` 已由骨架固定，不需要再次申请。

## 禁止自行修改

```text
src/core/
src/mapping/
src/replacement/
B5CacheVisualizer/
其他成员测试文件
```

## Statistics 输入与输出

输入是每次核心访问产生的：

```cpp
AccessResult
```

输出是只读：

```cpp
StatisticsSnapshot
```

固定关系：

```text
accesses = reads + writes
accesses = l1Hits + l2Hits + memoryMisses
overall hit rate = (l1Hits + l2Hits) / accesses
miss rate = memoryMisses / accesses
```

分母为 0 时所有 rate 返回 0.0。

## Trace 格式

必须支持：

```text
R 0x10
r 16
W 0x20
w 32
48
# comment
R 0x40 # inline comment
```

规则：

- R/r 为读；
- W/w 为写；
- 没有操作符时默认读；
- 支持十进制和 `0x` 十六进制；
- 空行与注释行忽略；
- 无效地址抛出异常；
- 异常信息必须包含行号；
- 文件导入读取文本后复用同一解析逻辑。

## 具体任务

### E1. 完善统计

- 复核 Record 每次只增加一次 accesses；
- 正确拆分 reads/writes；
- 正确拆分三种 AccessOutcome；
- Reset 清空全部字段；
- 所有 rate 的空输入行为为 0；
- 复核骨架提供的 Miss Rate；

### E2. 完善文本 Trace

- 加强首尾空格、大小写、注释和异常输入处理；
- 拒绝多余的无法识别 token；
- 错误信息包含准确行号和错误 token；
- 大地址使用 `std::uint64_t`，不能截断为 32 位。

### E3. 文件导入服务

已经固定的公共接口：

```cpp
static std::vector<MemoryAccess> ParseFile(
    const std::filesystem::path& path);
```

直接实现现有 `ParseFile()`，不要修改签名。实现要求：

- 文件不存在时给出明确异常；
- 文件无法读取时给出明确异常；
- 读取后调用 `ParseText`；
- 不在 UI 目录复制解析实现。

## 必须测试

### StatisticsTests

至少覆盖：

1. 空统计；
2. 单次 L1 Hit；
3. 单次 L2 Hit；
4. 单次 Memory Miss；
5. 混合读写；
6. 各种 rate；
7. Reset；
8. 统计恒等式。

### TraceTests

至少覆盖：

1. 十进制；
2. 十六进制；
3. R/r/W/w；
4. 默认读；
5. 空行；
6. 整行注释；
7. 行尾注释；
8. 缺少地址；
9. 非法地址；
10. 多余 token；
11. 超出 uint64 范围；
12. 文件不存在；
13. 正常文件导入。

## 完成标准

- [ ] 统计恒等式始终成立；
- [ ] rate 无除零问题；
- [ ] Trace 正常格式全部支持；
- [ ] 错误格式包含行号；
- [ ] 文件导入复用 ParseText；
- [ ] StatisticsTests 与 TraceTests 完整；
- [ ] 所有自动测试通过；
- [ ] 未在 UI 中复制统计或解析代码。

## 建议提交信息

```text
Complete cache statistics and trace parsing
```

## 交给成员 E 的 AI 提示词

```text
你正在 B5CacheVisualizer 仓库中协助成员 E 完成 Statistics 和 Memory Trace 模块。

完整读取 AGENTS.md、docs/ARCHITECTURE.md、docs/INTERFACES.md、docs/OWNERSHIP.md、docs/tasks/E-statistics.md。先检查路径、git status、remote 和当前分支，必须位于 feature-statistics。

只修改 src/statistics/、src/trace/、StatisticsTests.cpp 和 TraceTests.cpp。需要继续修改 CacheTypes 或更改现有 ParseFile 签名前，先停止并生成公共接口修改申请。严格测试计数恒等式、空分母、十进制/十六进制、R/W、注释、行号错误和文件导入。最终运行 scripts/test.ps1，汇报测试结果，不修改 UI，不合并 dev/main。
```
