# 阶段 04 任务卡：局部性 Trace 生成器

## 阶段信息

```text
顺序：04
分支：feature-04-locality-trace-generator
主题：可复现的教学型访存序列生成
前置：阶段 03 已合入 dev；开始编码前先把最新 origin/dev 合入本分支
状态：已实现并通过开发侧测试，待 PR 合并与组长复验
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

## 本分支实现与验收记录

实现新增了独立的 `TraceGenerator`：五种模式统一返回 `std::vector<MemoryAccess>`，再格式化为已有解析器可直接读取的标准文本。随机、热点和混合读写模式均使用用户可见的显式种子；请求数、范围、步长、概率和 `uint64_t` 地址溢出均在写入编辑框前校验。

界面在主窗口 Trace 区增加生成器入口，并使用独立参数对话框容纳五种模式、四个教学预设和“Generate + Load”。主窗口保持在 1366 x 768 屏幕内完整显示；生成成功后会停止播放、替换 Trace、清除旧帧/统计/高亮，并可立即使用现有 Next、Run All 和 Auto 播放。失败时生成器对话框保持打开，原 Trace 与当前会话不变。

开发侧回归已确认：Debug x64 编译 0 警告、0 错误，65/65 自动测试通过；生成文本使用 Windows CRLF 换行，MFC 编辑框中每条访问独占一行。参数读取只校验当前模式需要的字段，已禁用的无关字段不会阻止生成。人工测试清单已补充 UI-29 至 UI-34，供组长最终复验。

提交 PR 前的手工 UI 验收：

1. 选择 `Sequential locality` 预设并点击 `Generate + Load`；Trace 编辑框应得到 32 条 `R 0x...`，然后可点击 Next。
2. 选择 `Loop working set`；生成的前四个地址应以 16 字节为间隔，并从第五条开始重复。
3. 选择 `Hot access` 或 `Mixed reads/writes`；保持参数不变，连续生成两次，文本应完全相同；Mixed 预设应同时出现 `R` 与 `W`。
4. 先点击 Auto，再点击 Stop；随后生成任意预设。状态应回到 `Stopped F0/0`，统计、图表和两级表格均清空。
5. 将 Count 设为 `0`，或将 Start 设为 `0xFFFFFFFFFFFFFFFF`、Range 设为 `2`；应显示错误提示，且编辑框中的原 Trace 不变。
