# B5CacheVisualizer 总体架构

## 1. 项目目标

本项目是 Windows + Visual Studio 2022 + C++17 + MFC 的多级 Cache 映射与替换算法可视化演示系统。最终需要支持：

- L1 / L2 Cache；
- Cache Size、Block Size、相联度配置；
- Direct Mapping、Fully Associative、Set Associative；
- FIFO、LRU；
- 文本或文件形式的 Memory Trace；
- Hit、Miss、Hit Rate、Miss Rate；
- Cache Line 状态与替换过程可视化。

## 2. 架构原则

项目采用“核心模拟器与 MFC 界面分离”的结构：

```text
MFC UI
  │  只负责读取输入、调用接口、显示结果
  ▼
CacheSimulator
  ├── CacheLevel（L1 / L2 状态与访问流程）
  ├── IMappingStrategy（地址映射）
  ├── IReplacementPolicy（替换策略）
  ├── CacheStatistics（统计）
  └── MemoryTraceParser（访存序列解析）
```

必须保持依赖方向从 UI 指向核心，核心不得依赖 MFC：

```text
B5CacheVisualizer/  →  src/
src/core/           →  common + mapping + replacement + statistics
src/mapping/        →  common
src/replacement/    →  common
src/statistics/     →  common
src/trace/          →  common
```

`src/` 中不得包含 `CString`、`CDialogEx`、Windows 控件 ID 或其他 MFC 类型。这样核心模块可以由控制台测试工程独立编译。

## 3. 目录结构

```text
B5CacheVisualizer/
├── AGENTS.md                         AI 的全仓库约束
├── B5CacheVisualizer.sln             Visual Studio 解决方案
├── B5CacheVisualizer/                MFC 应用项目（成员 D）
├── B5CacheCoreTests/                 无 MFC 的核心测试项目
├── src/
│   ├── common/                       公共数据结构，组长控制
│   ├── core/                         L1/L2 模拟流程（成员 A）
│   ├── mapping/                      地址映射（成员 B）
│   ├── replacement/                  FIFO/LRU（成员 C）
│   ├── statistics/                   统计（成员 E）
│   └── trace/                        Trace 解析（成员 E）
├── docs/
│   ├── tasks/                        A–E 个人任务卡
│   ├── ARCHITECTURE.md               本文档
│   ├── INTERFACES.md                 公共接口约定
│   └── OWNERSHIP.md                  文件归属和合并顺序
└── scripts/                          固定构建与测试命令
```

## 4. 一次访存的标准流程

`CacheSimulator::Access()` 是 UI 发起一次访存的唯一入口：

```text
MemoryAccess(address, isWrite)
        ↓
按 L1 block size 计算 L1 block number
        ↓
Probe L1
  ├── Hit：更新命中信息和统计，返回 L1Hit
  └── Miss：继续访问 L2
                ↓
       按 L2 block size 计算 L2 block number
                ↓
             Probe L2
        ├── Hit：填充 L1，返回 L2Hit
        └── Miss：填充 L2 和 L1，返回 MemoryMiss
                ↓
        CacheStatistics::Record(result)
                ↓
            返回 AccessResult
```

当前骨架已经实现能够运行的 Direct Mapping + FIFO 基线。下列内容被有意保留给成员完成：

- `TODO(B)`：Fully Associative 和 Set Associative；
- `TODO(C)`：LRU；
- `TODO(A)`：最终的多级一致性、写回与包含策略；
- `TODO(D)`：完整配置界面、Cache Line 网格和单步可视化；
- `TODO(E)`：完整统计展示、Trace 文件导入和异常输入覆盖。

未实现模块会明确抛出异常或保留 TODO，不会静默返回伪造结果。

## 5. 当前基础模型

当前骨架采用以下基础约定：

- 地址按字节寻址；
- `blockNumber = address / blockSizeBytes`；
- L1 与 L2 可以使用不同 Block Size；
- 读写请求统一通过 `MemoryAccess`；
- 写命中会把对应 Cache Line 标记为 dirty；
- 写不命中暂按 write-allocate 的框架路径填充；
- `CacheSimulator::Reset()` 同时清空 L1、L2 和统计；
- 模拟器为单线程模型；
- 每次访问使用递增 tick，供 FIFO/LRU 判断时间顺序。

最终 write-back、L2 驱逐对 L1 的影响以及包含性策略由成员 A 按任务卡实现，并在修改公共结果结构前经过组长确认。

## 6. 为什么把策略做成接口

映射和替换采用接口 + 工厂方式：

```cpp
CreateMappingStrategy(MappingKind::Direct);
CreateReplacementPolicy(ReplacementKind::Fifo);
```

这样 `CacheLevel` 只依赖统一接口，不需要在核心流程中散落大量 `if (mapping == ...)`。成员 B 和 C 可以只修改各自目录，不需要改 MFC UI。

## 7. UI 与核心的边界

UI 可以做：

- 读取用户配置；
- 解析按钮事件；
- 调用 `MemoryTraceParser` 与 `CacheSimulator`；
- 读取 `AccessResult`、`StatisticsSnapshot`、`L1().Sets()` 和 `L2().Sets()`；
- 把状态转换为 MFC 控件显示。

UI 不可以做：

- 自己计算 set index 或 tag；
- 自己判断 FIFO/LRU victim；
- 自己维护命中次数；
- 在 `Dlg.cpp` 中复制一套 Cache 状态；
- 通过修改核心私有成员来驱动显示。

若 UI 缺少某项只读数据，应先提出公共接口需求，由组长评估，而不是在 UI 中重新实现算法。

## 8. 测试结构

`B5CacheCoreTests` 不依赖 MFC，各模块拥有独立测试文件：

```text
CoreTests.cpp          成员 A
MappingTests.cpp       成员 B
ReplacementTests.cpp   成员 C
StatisticsTests.cpp    成员 E
TraceTests.cpp         成员 E
TestMain.cpp            公共测试入口，普通成员不修改
```

成员 D 使用 UI 人工测试清单，并保证核心测试仍全部通过。

## 9. 集成原则

- 五个人从同一个骨架版本创建分支；
- 每个人主要修改自己的目录和测试文件；
- 公共结构修改必须先讨论；
- PR 目标只能是 `dev`；
- 组长按 `B → C → E → A → D` 的建议顺序集成；
- 每次合并后重新运行整个测试项目；
- 全部集成稳定后，再从 `dev` 向 `main` 合并阶段版本。
