# 成员 D 任务卡：MFC UI 与可视化

## 身份信息

```text
成员代号：D
负责模块：MFC 界面、配置输入、Cache Line 可视化、单步演示
个人分支：feature-ui
前置条件：已确认代号 D；从最新 dev 创建 feature-ui；工作区状态已检查
```

## 允许主要修改的文件

```text
B5CacheVisualizer/B5CacheVisualizerDlg.h
B5CacheVisualizer/B5CacheVisualizerDlg.cpp
B5CacheVisualizer/B5CacheVisualizer.rc
B5CacheVisualizer/resource.h
docs/testing/UI_TEST_CHECKLIST.md
```

需要新增 UI 类或修改 `.vcxproj` 时，先在群里通知组长，避免项目文件冲突。

## 禁止自行修改

```text
src/core/
src/mapping/
src/replacement/
src/statistics/
src/trace/
src/common/
B5CacheCoreTests/
```

UI 中禁止重新实现映射、替换或命中率计算。

## 当前基础

骨架已有一个可启动对话框：

- Trace 多行输入框；
- Run Trace 按钮；
- Reset 按钮；
- 基础统计文本；
- 调用 `MemoryTraceParser` 和 `CacheSimulator` 的示例。

成员 D 在这一基础上完善，不要重新创建第二个 MFC 工程。

## 最终界面必须包含

### 配置区域

L1 和 L2 各自显示：

- Cache Size；
- Block Size；
- Associativity；
- Mapping 下拉框；
- Replacement 下拉框。

应用配置时构造新的 `CacheSimulator`。如果配置非法，捕获异常并向用户显示可理解的错误，不要让程序崩溃。

### Trace 区域

- 多行文本输入；
- 支持 `R 0x10`、`W 32` 等格式提示；
- 导入文件按钮；
- 清空按钮；
- 显示当前将执行到第几条。

文件读取逻辑优先调用成员 E 提供的 Trace 接口；不要在 UI 中复制解析规则。

### 控制区域

- Apply Config；
- Step：每次只调用一次 `simulator.Access()`；
- Run All：执行剩余 trace；
- Reset：清空模拟状态并回到第一条；
- 必要时提供速度或“下一步”提示，但不要先做复杂动画。

### Cache Line 可视化区域

L1 和 L2 分开显示，至少包含列：

```text
Set
Line
Valid
Dirty
Tag
Block
Inserted At
Last Used At
```

每次 Step 后刷新，使用 `simulator.L1().Sets()` 和 `simulator.L2().Sets()`。根据 `AccessResult` 高亮：

- 命中的 line；
- 新填入的 line；
- 被替换的 line；
- 当前结果是 L1 Hit、L2 Hit 或 Memory Miss。

### 统计区域

读取 `simulator.Statistics()` 显示：

- Accesses；
- Reads；
- Writes；
- L1 Hits；
- L2 Hits；
- Memory Misses；
- L1 Hit Rate；
- L2 Hit Rate；
- Overall Hit Rate；
- Miss Rate。

## 代码结构要求

`B5CacheVisualizerDlg.cpp` 不应无限膨胀。至少把逻辑拆成清楚的私有方法，例如：

```cpp
bool ReadConfiguration(...);
bool LoadTraceFromEditor(...);
void ResetSession();
void ExecuteNextAccess();
void RefreshCacheViews(const AccessResult* latest);
void RefreshStatistics();
void ShowUserError(...);
```

如果需要新增 ViewModel/Formatter 类，先告知组长并让 AI 更新项目文件。

## 人工测试清单

在 `docs/testing/UI_TEST_CHECKLIST.md` 中逐项记录：

1. 程序启动；
2. 默认配置运行；
3. 非法 Size；
4. 非法 Block Size；
5. Direct Mapping；
6. Fully Associative；
7. Set Associative；
8. FIFO；
9. LRU；
10. Step；
11. Run All；
12. Reset；
13. 空 Trace；
14. 错误 Trace；
15. 文件导入；
16. L1/L2 line 刷新；
17. hit/miss 高亮；
18. 窗口缩放或最小尺寸下不遮挡关键控件。

每项写明操作、期望和实际结果。

## 完成标准

- [ ] 用户可以配置 L1/L2；
- [ ] 用户可以输入或导入 trace；
- [ ] Step 和 Run All 正常；
- [ ] L1/L2 line 状态可见；
- [ ] 当前命中/替换过程有清楚高亮；
- [ ] 统计完整；
- [ ] 非法输入不会导致崩溃；
- [ ] UI 没有重复实现核心算法；
- [ ] 核心自动测试仍全部通过；
- [ ] UI 人工测试清单填写完整。

## 建议提交信息

```text
Add interactive cache visualization UI
```

## 交给成员 D 的 AI 提示词

```text
你正在 B5CacheVisualizer 仓库中协助成员 D 完成 MFC UI。

完整读取 AGENTS.md、docs/ARCHITECTURE.md、docs/INTERFACES.md、docs/OWNERSHIP.md、docs/tasks/D-ui.md。先检查仓库路径、git status、远程地址和当前分支；必须位于 feature-ui。

只修改任务卡允许的 MFC UI 文件和 UI 测试清单。UI 只读取输入、调用 CacheSimulator/MemoryTraceParser、展示 AccessResult/StatisticsSnapshot/Sets，不在 Dlg.cpp 中实现映射、替换或统计。新增文件或修改项目文件前先停止并生成给组长的说明。

分阶段实现配置、Trace、Step/Run、Cache 网格、统计和错误处理，每阶段编译。最终运行 scripts/test.ps1，并给出详细人工 UI 测试结果。不要合并 dev/main。
```
