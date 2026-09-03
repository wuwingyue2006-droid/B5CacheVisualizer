# 阶段 05 任务卡：策略对比实验

## 阶段信息

```text
顺序：05
分支：feature-05-strategy-comparison
主题：同一 Trace 下的多配置并行实验与可视化对比
前置：阶段 04 已合入 dev；开始编码前先把最新 origin/dev 合入本分支
状态：待开发
```

本阶段把程序从单次模拟器提升为教学型实验比较平台：用户保存 2～3 套 Cache 配置，对同一条 Trace 独立运行，并在一个界面中比较结果。

## 开发目标

- 支持保存、编辑和删除 2～3 个实验方案，方案名称必须可区分。
- 每个方案完整保存 L1/L2 的 Size、Block Size、Associativity、Mapping 和 Replacement。
- 所有方案使用同一份已经解析成功的 Trace，彼此使用独立 `CacheSimulator`，不共享可写状态。
- 输出对比表：Accesses、L1 Hits、L2 Hits、Memory Misses、L1 Hit Rate、Overall Hit Rate、Miss Rate。
- 输出对比图，至少直观比较 Overall Hit Rate 与 Miss Rate。
- 提供一键加载的典型方案：Direct + FIFO、2-way Set + LRU、Fully Associative + LRU。
- 提供一条能体现 Direct、Set 和 Fully 差异的内置 Trace，并注明其教学目的。
- 可以把某个方案载入主界面，继续使用已有单步和动画展示。

## 建议设计

把实验运行逻辑放在不依赖 MFC 的纯 C++ 服务中，例如：

```text
src/experiment/ComparisonRunner.h
src/experiment/ComparisonRunner.cpp
```

建议使用以下内部模型：

```text
ComparisonPlan
  - name
  - SimulationConfig

ComparisonResult
  - plan name
  - StatisticsSnapshot
  - optional access results
```

`ComparisonRunner` 只负责为每个方案新建模拟器、执行同一组 `MemoryAccess` 并收集结果。它不得在外部修改主界面的 `simulator_`，也不得重新实现命中率公式。

## 建议文件边界

允许主要新增或修改：

```text
src/experiment/ComparisonRunner.h
src/experiment/ComparisonRunner.cpp
B5CacheVisualizer/ComparisonDlg.h
B5CacheVisualizer/ComparisonDlg.cpp
B5CacheVisualizer/B5CacheVisualizerDlg.h
B5CacheVisualizer/B5CacheVisualizerDlg.cpp
B5CacheVisualizer/B5CacheVisualizer.rc
B5CacheVisualizer/resource.h
B5CacheVisualizer/B5CacheVisualizer.vcxproj
B5CacheCoreTests/B5CacheCoreTests.vcxproj
docs/tasks/05-strategy-comparison.md
```

对比模块通过现有 `SimulationConfig`、`MemoryAccess`、`CacheSimulator` 和 `StatisticsSnapshot` 工作。若现有只读接口足够，不得修改 `src/common/CacheTypes.h` 或 `docs/INTERFACES.md`。

## 实现顺序

1. 定义实验方案和结果模型，完成独立运行服务。
2. 复用核心构造阶段的配置校验，任一方案非法时指出具体方案与 L1/L2 错误。
3. 设计独立对比窗口，避免继续挤压主对话框固定布局。
4. 实现方案的新增、编辑、删除以及 2～3 个槽位管理。
5. 解析一次当前 Trace；成功后把同一请求数组传给每个方案。
6. 实现对比表，统一百分比精度与主界面 Statistics 显示。
7. 使用现有 GDI/GDI+ 风格绘制对比条形图，不引入大型第三方图表库。
8. 添加三组典型配置预设和一条冲突 Miss 教学 Trace。
9. 添加“载入主界面”操作：停止播放、应用选中配置、载入同一 Trace 并重置会话。

## 数据一致性要求

- 每次点击运行都重新创建独立模拟器，不能复用上一次残留状态。
- 方案执行顺序不能影响任何结果。
- 表格和图表必须来自同一个 `ComparisonResult`，不能分别计算。
- 百分比只在展示层格式化，不在 UI 中复制统计公式。
- 内置配置必须满足三种映射模式对 associativity 的既有约束。
- 一个方案失败时不得把其他方案的旧结果伪装成本轮结果。

## 不属于本阶段

- 不新增 L3、Victim Cache、预取器或多核一致性；
- 不实现新的映射、替换或写策略；
- 不开发 CSV/TXT 文件导出；
- 不修改主模拟器的核心行为；
- 不处理最终 Release、跨电脑、打包和集中验收。

## 开发完成判定

- 可以配置并运行 2～3 个互相独立的方案；
- 所有方案使用完全相同的 Trace；
- 表格与图表清楚展示策略差异；
- 三个教学预设与内置 Trace 可以一键使用；
- 选中方案可以安全载入主界面继续单步演示；
- 对比运行服务与 MFC 界面分离，核心公共接口保持不变。

## 建议提交信息

```text
Add cache strategy comparison experiments
```
