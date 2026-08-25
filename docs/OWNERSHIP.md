# B5CacheVisualizer 文件归属与集成顺序

## 1. 五人模块分配

| 成员 | 分支 | 模块 | 主要目录 | 独立测试文件 |
|---|---|---|---|---|
| A | `feature-core` | Core / 多级流程 | `src/core/` | `CoreTests.cpp` |
| B | `feature-mapping` | Mapping | `src/mapping/` | `MappingTests.cpp` |
| C | `feature-replacement` | Replacement | `src/replacement/` | `ReplacementTests.cpp` |
| D | `feature-ui` | MFC UI | `B5CacheVisualizer/` | 人工 UI 测试清单 |
| E | `feature-statistics` | Statistics + Trace | `src/statistics/`、`src/trace/` | `StatisticsTests.cpp`、`TraceTests.cpp` |

这里的 A–E 是成员代号，之后由组长把真实姓名填入各自任务卡副本。

## 2. 公共文件

下列文件不属于任何成员可以随意修改的私人区域：

```text
src/common/CacheTypes.h
src/common/CacheTypes.cpp
docs/INTERFACES.md
B5CacheVisualizer.sln
B5CacheVisualizer/*.vcxproj
B5CacheCoreTests/*.vcxproj
B5CacheCoreTests/TestMain.cpp
B5CacheCoreTests/TestSupport.h
B5CacheCoreTests/TestSuites.h
AGENTS.md
```

需要修改这些文件时，先提交公共接口修改申请，由组长决定谁修改以及何时合入。

## 3. 成员主要修改范围

### 成员 A

允许主要修改：

```text
src/core/CacheLevel.h
src/core/CacheLevel.cpp
src/core/CacheSimulator.h
src/core/CacheSimulator.cpp
B5CacheCoreTests/CoreTests.cpp
```

### 成员 B

允许主要修改：

```text
src/mapping/IMappingStrategy.h
src/mapping/MappingStrategies.cpp
B5CacheCoreTests/MappingTests.cpp
```

### 成员 C

允许主要修改：

```text
src/replacement/IReplacementPolicy.h
src/replacement/ReplacementPolicies.cpp
B5CacheCoreTests/ReplacementTests.cpp
```

### 成员 D

允许主要修改：

```text
B5CacheVisualizer/B5CacheVisualizerDlg.h
B5CacheVisualizer/B5CacheVisualizerDlg.cpp
B5CacheVisualizer/B5CacheVisualizer.rc
B5CacheVisualizer/resource.h
docs/testing/UI_TEST_CHECKLIST.md
```

新增 UI 类需要修改 `.vcxproj` 时，先通知组长。

### 成员 E

允许主要修改：

```text
src/statistics/CacheStatistics.h
src/statistics/CacheStatistics.cpp
src/trace/MemoryTraceParser.h
src/trace/MemoryTraceParser.cpp
B5CacheCoreTests/StatisticsTests.cpp
B5CacheCoreTests/TraceTests.cpp
```

## 4. 合并建议顺序

建议组长按以下顺序向 `dev` 合并：

```text
1. feature-mapping
2. feature-replacement
3. feature-statistics
4. feature-core
5. feature-ui
```

原因：

- B、C、E 主要实现独立策略和服务；
- A 的核心流程会使用 B、C、E 的完成版本；
- D 最后连接完整功能并完成可视化，最容易发现集成问题。

D 不需要等到最后才开始工作，可以使用骨架并行开发；这里只表示最终 PR 的建议合入顺序。

## 5. 出现交叉修改时

如果成员发现任务必须修改他人文件：

1. 先停止修改；
2. 说明为什么本模块无法只通过公共接口完成；
3. 列出希望修改的文件和具体行；
4. 由文件负责人或组长完成该修改；
5. 该成员更新 `dev` 后继续。

禁止让 AI 为了“方便”直接重构整个仓库。
