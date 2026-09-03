# 阶段 04 任务卡：局部性 Trace 生成器

## 阶段信息

```text
顺序：04
分支：feature-04-locality-trace-generator
主题：可复现的教学型访存序列生成
前置：阶段 03 已合入 dev；开始编码前先把最新 origin/dev 合入本分支
状态：待开发
```

本阶段提供可以直接进入现有 Trace 编辑框并由 `MemoryTraceParser` 执行的访存序列，用来直观演示时间局部性、空间局部性、冲突 Miss 和读写混合。

## 开发目标

提供五种生成模式：

1. `Sequential`：从起始地址按步长顺序生成。
2. `Loop`：在指定地址范围内循环访问。
3. `Random`：在指定范围内均匀随机访问。
4. `Hot Set`：大部分请求落在热点集合，少量请求访问冷区。
5. `Mixed R/W`：按给定写比例生成读写混合请求。

所有模式至少支持请求数量、起始地址和步长；涉及随机性的模式必须支持显式随机种子，同一组参数与种子必须得到完全一致的结果。

## 建议设计

把生成算法放在不依赖 MFC 的纯 C++ 模块中，例如：

```text
src/trace/TraceGenerator.h
src/trace/TraceGenerator.cpp
```

建议输入结构包含：

```text
模式
请求数量
起始地址
地址范围
步长
循环长度
热点数量或热点范围
热点概率
写入比例
随机种子
```

返回 `std::vector<MemoryAccess>`；由单独的格式化方法生成规范文本，例如 `R 0x10`、`W 0x20`。UI 只读取参数、调用生成器并把文本写入 Trace 编辑框，不在按钮事件中实现随机算法。

## 建议文件边界

允许主要新增或修改：

```text
src/trace/TraceGenerator.h
src/trace/TraceGenerator.cpp
B5CacheVisualizer/B5CacheVisualizerDlg.h
B5CacheVisualizer/B5CacheVisualizerDlg.cpp
B5CacheVisualizer/B5CacheVisualizer.rc
B5CacheVisualizer/resource.h
B5CacheVisualizer/B5CacheVisualizer.vcxproj
B5CacheCoreTests/B5CacheCoreTests.vcxproj
docs/tasks/04-locality-trace-generator.md
```

如新增独立参数对话框，可以增加 `TraceGeneratorDlg.h/.cpp` 并同步工程文件。不得修改现有 `MemoryTraceParser` 的语法规则，不得修改 Cache 模拟算法。

## 实现顺序

1. 定义生成模式和参数结构，集中进行正数、范围、概率及地址溢出校验。
2. 完成 `Sequential` 和 `Loop`，确认地址计算不会越过 `uint64_t`。
3. 使用固定算法和显式种子完成 `Random`、`Hot Set`、`Mixed R/W`。
4. 实现统一文本格式化，保证输出可被现有解析器直接读取。
5. 添加参数界面与模式切换；隐藏或禁用当前模式不需要的参数。
6. 添加“生成并载入”，以一次操作替换 Trace 编辑框内容。
7. 生成新 Trace 前停止阶段 03 的播放，清空旧时间线、统计、图表与高亮。
8. 增加少量教学预设：顺序局部性、循环工作集、热点访问和读写混合。

## 参数规则

- 请求数量必须大于 0，并设置合理上限，防止误生成超大文本冻结 UI。
- 步长与范围必须能够形成有效地址序列。
- 概率统一使用清晰的百分比或 `0～1` 表示，界面与代码含义必须一致。
- 写比例为 0 时全部生成读；为 100% 时全部生成写。
- 发生地址加法或乘法溢出时，向用户显示明确错误，不进行截断。
- 生成器不得依赖系统当前时间作为唯一种子；用户必须能看到并复用种子。

## 不属于本阶段

- 不同时运行多组 Cache 配置；
- 不输出策略对比表或图；
- 不导出 CSV/TXT 文件；
- 不修改 Trace 解析格式和 Cache 核心接口；
- 不处理最终 Release、跨电脑、打包和集中验收。

## 开发完成判定

- 五种模式都能生成规范 Trace 文本并载入编辑框；
- 相同随机种子与参数产生相同文本；
- 生成结果可以沿用现有 Step、Run All 与动画控制器；
- 参数非法或地址溢出时只显示错误，不破坏当前会话；
- 生成新 Trace 后旧动画状态被正确清除；
- 生成算法与 MFC UI 保持分离。

## 建议提交信息

```text
Add locality trace generator
```
