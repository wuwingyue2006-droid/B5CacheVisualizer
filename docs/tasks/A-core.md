# 成员 A 任务卡：技术组长、Core 与集成

## 身份信息

```text
成员代号：A
团队身份：技术组长
负责模块：总体架构 / 公共接口 / Core / L1-L2 访问流程 / 配置校验 / PR 审核 / 系统集成
个人分支：feature-core
前置条件：已确认代号 A；从最新 dev 创建 feature-core；工作区状态已检查
```

## 允许主要修改的文件

```text
src/core/CacheLevel.h
src/core/CacheLevel.cpp
src/core/CacheSimulator.h
src/core/CacheSimulator.cpp
B5CacheCoreTests/CoreTests.cpp
```

## 未经组长同意不要修改

```text
src/common/
src/mapping/
src/replacement/
src/statistics/
src/trace/
B5CacheVisualizer/
docs/INTERFACES.md
解决方案和项目文件
其他成员测试文件
```

## 当前基础

骨架已经提供：

- `CacheLevel` 的 Probe、Insert、Reset；
- `CacheSimulator` 的 Access、Run、Reset；
- 默认 L1/L2 配置；
- Direct Mapping + FIFO 下可运行的 L1 Hit、L2 Hit、Memory Miss 流程；
- Cache Line 只读状态；
- 基础 Core 测试。

不要重新创建另一套 Cache 类，也不要把整个核心重写到 UI 中。

## 本模块目标

完成稳定、可测试、可供 UI 单步调用的两级 Cache 调度核心。

除本模块开发外，A 还负责检查 B–E 的 PR、控制公共接口修改、处理合并冲突、按建议顺序合入 `dev`、运行全量测试，并在系统稳定后组织 `dev → main` 的最终发布。具体组长职责见 `docs/B5五人精确分工与组长职责.md`。

本项目采用简化教学模型：

- L1、L2 使用各自配置独立映射；
- L1 miss 后访问 L2；
- L2 hit 或 Memory miss 后填充 L1；
- Memory miss 时先填充 L2；
- write miss 使用 write-allocate；
- dirty 位用于状态展示；
- 暂不统计真实内存写回时延；
- 不强制 L1/L2 inclusive；
- 每次 `Access()` 就是 UI 的一个可视化步骤。

不要擅自扩展成复杂硬件一致性协议。

## 具体任务

### A1. 完善配置校验

确认并测试：

- Size、Block Size 不为 0；
- Size 能被 Block Size 整除；
- associativity 合法；
- Direct、Fully、Set Associative 的相联度规则正确；
- 错误信息包含 L1 或 L2 名称；
- 非法配置在构造阶段抛出 `std::invalid_argument`。

建议增加 2 的幂校验前，先向组长确认课程是否允许非 2 的幂配置；没有确认不要擅自增加限制。

### A2. 核对一次访问的结果

对每种结果检查：

- L1 Hit 不应访问或修改 L2；
- L2 Hit 应向 L1 填入对应 block；
- Memory Miss 应填充 L2 和 L1；
- `LevelAccessDetail` 的 hit、setIndex、lineIndex、evicted、evictedBlock 正确；
- 写命中/写填充后 dirty 为 true；
- tick 每次 Access 只递增一次；
- 每次 Access 只记录一次统计。

### A3. 保证 Reset 完整

Reset 后：

- L1 全部 invalid；
- L2 全部 invalid；
- dirty 清空；
- FIFO/LRU 时间清空；
- tick 回到初始状态；
- 统计全部为 0。

### A4. 增加核心测试

只修改 `CoreTests.cpp`，至少覆盖：

1. 第一次访问是 Memory Miss；
2. 重复访问是 L1 Hit；
3. L1 被驱逐但 L2 保留时是 L2 Hit；
4. 读写计数正确；
5. 写入后 dirty 正确；
6. L1/L2 evicted 信息正确；
7. Reset 后再次访问重新 miss；
8. L1 非法配置；
9. L2 非法配置；
10. 多条 trace 的结果顺序与输入一致。

不要在 CoreTests 中重复测试 B/C 算法的全部数学细节。

## 完成标准

- [ ] `TODO(A)` 已按上述简化模型处理或更新为准确说明；
- [ ] Direct + FIFO 基线行为没有退化；
- [ ] B、C 完成后 Core 可通过工厂使用三种映射和两种替换；
- [ ] AccessResult 各字段有测试；
- [ ] Reset 有完整测试；
- [ ] 所有自动测试通过；
- [ ] 未修改 UI 算法；
- [ ] 未擅自改变公共类型。

## 建议提交信息

```text
Complete two-level cache simulation flow
```

## 交给成员 A 的 AI 提示词

```text
你正在 B5CacheVisualizer 仓库中协助成员 A 完成 Core 模块。

开始前必须完整读取：
AGENTS.md
docs/ARCHITECTURE.md
docs/INTERFACES.md
docs/OWNERSHIP.md
docs/tasks/A-core.md

先运行 Get-Location、git status、git remote -v、git branch --show-current。
只有当前分支为 feature-core 且工作区状态已解释清楚时才能修改。

严格只修改 A 任务卡允许的 Core 文件和 CoreTests.cpp。不要重写 Mapping、Replacement、Statistics、Trace 或 MFC UI。修改公共类型或接口前必须停止并生成公共接口修改申请，不得自行执行。

按 A1、A2、A3、A4 顺序完成，每完成一项先编译测试。最终运行 scripts/test.ps1，报告修改文件、测试结果、公共接口变化和建议 commit 信息。不要合并 dev/main，不要使用危险 Git 命令。
```
