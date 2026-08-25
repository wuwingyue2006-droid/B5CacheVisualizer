# 成员 C 任务卡：Replacement

## 身份信息

```text
成员代号：C
负责模块：FIFO / LRU 替换策略
个人分支：feature-replacement
前置条件：已确认代号 C；从最新 dev 创建 feature-replacement；工作区状态已检查
```

## 允许主要修改的文件

```text
src/replacement/IReplacementPolicy.h
src/replacement/ReplacementPolicies.cpp
B5CacheCoreTests/ReplacementTests.cpp
```

一般不需要修改接口头文件。

## 禁止自行修改

```text
src/core/
src/mapping/
src/statistics/
src/trace/
B5CacheVisualizer/
src/common/CacheTypes.h
其他成员测试文件
```

## 已确定接口行为

`CacheLevel` 会先寻找 invalid line，只有整组全部 valid 时才调用：

```cpp
SelectVictim(const std::vector<CacheLineState>& lines)
```

因此成员 C 不负责查找空 line，只负责在给定的 valid lines 中选择 victim。

时间字段：

```text
insertedAt：该 block 最近一次进入此 line 的 tick
lastUsedAt：该 line 最近一次被访问或填充的 tick
```

## FIFO 规则

- 选择 `insertedAt` 最小的 line；
- Hit 时不能改变 `insertedAt`；
- Hit 可以更新 `lastUsedAt`，便于 UI 展示；
- Insert 时同时设置 insertedAt 和 lastUsedAt；
- 时间相同选择较小下标。

骨架已有 FIFO 基线，成员 C 负责补齐边界测试。

## LRU 规则

- 选择 `lastUsedAt` 最小的 line；
- 每次 Hit 更新 `lastUsedAt = tick`；
- 每次 Insert 设置 `insertedAt = tick`、`lastUsedAt = tick`；
- 时间相同选择较小下标；
- 不得使用系统时间，必须使用核心传入的 tick。

## 具体任务

1. 实现 `LruReplacementPolicy::SelectVictim()`；
2. 复核 LRU OnHit 和 OnInsert；
3. 复核 FIFO 不会因 Hit 改变插入顺序；
4. 空 lines 抛出 `std::invalid_argument`；
5. 移除完成部分的 `TODO(C)` 和 `logic_error`；
6. 不在策略内部访问 CacheSimulator 或 UI。

## 必须测试

在 `ReplacementTests.cpp` 中至少覆盖：

1. FIFO 选择最早插入项；
2. FIFO 命中后 victim 不变；
3. FIFO 插入后 timestamp 正确；
4. LRU 选择最久未使用项；
5. LRU 命中后该 line 不再是 victim；
6. LRU 插入后 timestamp 正确；
7. 相同 timestamp 选择较小下标；
8. 只有一个 line 时返回 0；
9. 空集合抛出异常。

测试只操作策略和 CacheLineState，不需要创建 MFC 窗口。

## 完成标准

- [ ] FIFO/LRU 均可通过工厂创建；
- [ ] LRU 行为符合上述规则；
- [ ] FIFO 原有行为没有退化；
- [ ] 决策结果稳定可复现；
- [ ] `TODO(C)` 已移除；
- [ ] ReplacementTests 覆盖边界情况；
- [ ] 完整测试全部通过；
- [ ] 未修改 Core 来绕过策略接口。

## 建议提交信息

```text
Implement LRU replacement policy
```

## 交给成员 C 的 AI 提示词

```text
你正在 B5CacheVisualizer 仓库中协助成员 C 完成 Replacement 模块。

完整读取 AGENTS.md、docs/ARCHITECTURE.md、docs/INTERFACES.md、docs/OWNERSHIP.md、docs/tasks/C-replacement.md。先检查路径、远程仓库、git status 和当前分支；当前必须是 feature-replacement。

只修改 src/replacement/ 和 B5CacheCoreTests/ReplacementTests.cpp。严格按任务卡的 insertedAt、lastUsedAt 和稳定 tie-break 规则实现 FIFO/LRU。不要改 Core、Mapping、UI 或公共类型。完成后运行 scripts/test.ps1，汇报测试和 commit 建议，不合并 dev/main。
```
