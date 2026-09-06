# 阶段 06 任务卡：实验结果导出（可选）

## 阶段信息

```text
顺序：06
分支：feature-06-result-export
主题：把单次模拟与策略对比结果导出为可复用实验材料
前置：阶段 05 已合入 dev；开始编码前先把最新 origin/dev 合入本分支
优先级：可选，时间不足时可以取消
状态：已修复并通过开发侧自动化验收，待 PR 合入与组长 UI 复验
```

本阶段只负责把程序已经产生的数据整理为文件，不增加新的 Cache 模型。导出内容应便于写实验报告、使用 Excel 查看以及复现课堂演示。

## 开发目标

至少提供两种输出：

- CSV：适合统计表、逐条访问记录和多方案对比。
- TXT：适合保存人类可读的配置、Trace 摘要与实验结论。

导出内容包括：

1. 导出时间、程序版本或阶段说明。
2. L1/L2 完整配置。
3. Trace 总条数、读写数量及原始 Trace 或规范化 Trace。
4. 每条请求的序号、R/W、地址、访问结果、L1/L2 set/line、驱逐信息。
5. 最终 `StatisticsSnapshot`。
6. 阶段 05 的多方案对比表。
7. 基于结果的简短客观结论，例如最高总体命中率方案；不得伪造原因分析。

## 建议设计

把格式化与文件写入放在不依赖 MFC 的纯 C++ 模块中，例如：

```text
src/export/ExperimentExporter.h
src/export/ExperimentExporter.cpp
```

导出器接收只读的配置、Trace、访问结果、统计和可选对比结果。MFC UI 只负责打开保存对话框、收集路径、选择格式、调用导出器以及显示成功或错误消息。

## 建议文件边界

允许主要新增或修改：

```text
src/export/ExperimentExporter.h
src/export/ExperimentExporter.cpp
B5CacheVisualizer/B5CacheVisualizerDlg.h
B5CacheVisualizer/B5CacheVisualizerDlg.cpp
B5CacheVisualizer/ComparisonDlg.h
B5CacheVisualizer/ComparisonDlg.cpp
B5CacheVisualizer/B5CacheVisualizer.rc
B5CacheVisualizer/resource.h
B5CacheVisualizer/B5CacheVisualizer.vcxproj
B5CacheCoreTests/B5CacheCoreTests.vcxproj
docs/tasks/06-result-export.md
```

如果阶段 05 的结果类型位于专用头文件，应直接复用；不得为了导出重复运行模拟器或复制统计公式。

## 实现顺序

1. 固定 CSV 列名、TXT 分节结构以及数值精度。
2. 实现内存中的 CSV/TXT 格式化，处理逗号、双引号、换行等转义。
3. 实现文件写入和错误传播，不能静默吞掉权限、路径或磁盘错误。
4. 统一输出 UTF-8；如为了 Windows Excel 兼容加入 UTF-8 BOM，应在代码和说明中保持一致。
5. 在主界面增加“导出当前实验”，仅在已有有效结果时启用。
6. 在对比窗口增加“导出对比结果”，复用同一导出模块。
7. 使用系统保存对话框提供 `.csv` / `.txt` 过滤器和合理默认文件名。
8. 导出成功后显示实际完整路径；取消保存不视为错误。

## 文件内容规则

- 地址统一使用带 `0x` 前缀的十六进制，同时可保留十进制列。
- 百分比展示精度与程序界面一致。
- 无效 line index 输出为空或 `N/A`，不能输出无符号最大值。
- L1 Hit 时清楚标记 L2 未访问。
- 只有核心结果报告了驱逐时才输出被驱逐 block。
- 所有 CSV 字段必须正确转义，确保配置名或备注含逗号时仍可读取。
- 不导出二进制对象、内存地址、临时目录或用户隐私路径。

## 不属于本阶段

- 不实现 PDF、Word 或图像报告；
- 不引入数据库或云端上传；
- 不新增 Cache 算法或统计指标；
- 不负责最终课程报告自动生成；
- 不处理 Release、跨电脑、最终 ZIP 和集中验收。

## 开发完成判定

- 当前单方案实验可导出 CSV 和 TXT；
- 多方案比较可导出统一对比表；
- 导出文件完整包含配置、Trace、逐条结果与最终统计；
- 特殊字符、中文方案名和取消保存都能被正确处理；
- 文件写入失败时用户能看到明确原因；
- 导出模块不依赖 MFC，且不修改核心模拟结果。

## 建议提交信息

```text
Add cache experiment result export
```

## 实现记录（2026-09-05）

- 新增 `src/export/ExperimentExporter.h/.cpp`：纯 C++、不依赖 MFC；接收只读的配置、Trace、逐条访问结果、最终统计和可选的对比结果。
- CSV 与 TXT 均输出 UTF-8 并统一带 UTF-8 BOM（`EF BB BF`），保证 Windows Excel 和记事本正确识别编码；由 `ExperimentExporter::WriteUtf8File` 统一写入，写入失败抛出含路径原因的异常，不静默吞错。
- 主界面新增"Export experiment..."按钮（`IDC_BUTTON_EXPORT`），仅在已执行至少一次访问后启用；对比窗口新增"Export results..."按钮（`IDC_BUTTON_COMPARE_EXPORT`），仅在运行对比后启用。保存对话框提供 `.csv` / `.txt` 过滤器与默认文件名，取消保存不视为错误。
- 地址统一 `0x` 十六进制并保留十进制列；无效 line index 输出为空或 `N/A`；L1 Hit 时 L2 各列留空并在 TXT 中标注 `not accessed (L1 hit)`；仅在实际发生驱逐时输出被驱逐 block；所有 CSV 字段正确转义。
- 新增 `B5CacheCoreTests/ExportTests.cpp`（6 个测试）并注册到 `TestSuites.h` / `TestMain.cpp`（这两个公共文件各加一行，与阶段 05 做法一致）。

## 补丁修复与验收（2026-09-06）

- 对比 CSV/TXT 现在同时导出完整的共享 Trace，包含序号、读写类型、十六进制地址和十进制地址，可直接复现本次多方案实验。
- 对比摘要的 `Executed accesses` 改为使用对比结果中的实际访问次数，不再因单实验逐条结果为空而错误显示为 `0`。
- 新增针对性自动测试，锁定对比 CSV/TXT 的完整 Trace 和正确执行次数；Debug x64 构建为 0 警告、0 错误，核心回归 75/75 通过。
