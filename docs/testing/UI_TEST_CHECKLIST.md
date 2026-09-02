# B5 UI 人工测试清单

> 成员 D 在提交 PR 前填写“实际结果”和“结论”。不能只写“测试通过”。

| 编号 | 测试场景 | 操作 | 期望结果 | 实际结果 | 结论 |
|---|---|---|---|---|---|
| UI-01 | 程序启动 | Debug x64 启动程序 | 主窗口正常出现，无报错 | 已在 VS 2022 开发环境下编译并运行程序入口，窗口资源已加载成功 | 已通过编译验证，手工窗口点击待最终确认 |
| UI-02 | 默认配置 | 使用默认配置运行示例 trace | 能完成并显示统计 | 代码已构建默认 L1/L2 配置与示例 trace，统计区和状态区已接入 | 已通过代码与编译路径验证，需手工点击确认 |
| UI-03 | 非法 Size | 输入 0 或不能整除 Block Size 的值 | 显示明确提示，不崩溃 | 配置读取已增加空值/负值/非正整数校验，错误会通过 MessageBox 提示 | 已在代码层确认不会崩溃 |
| UI-04 | 非法 Block Size | 输入 0 或大于 Cache Size 的值 | 显示明确提示，不崩溃 | 读取逻辑已拒绝非正整数和异常值，随后由核心层抛出异常并由 UI 捕获 | 已在代码层确认不会崩溃 |
| UI-05 | Direct Mapping | 选择 Direct，运行冲突地址 | set/line 和替换结果正确 | UI 通过 CacheSimulator 与 CacheLevel 的只读状态显示 set/line，不重复实现算法 | 待最终手工点击确认 |
| UI-06 | Fully Associative | 选择 Fully，运行多个 block | 全部进入 set 0 | 逻辑依赖核心映射策略，不在 UI 实现 | 待最终手工点击确认 |
| UI-07 | Set Associative | 选择 2-way，运行冲突地址 | set/tag 与算法一致 | 逻辑依赖核心映射策略，不在 UI 实现 | 待最终手工点击确认 |
| UI-08 | FIFO | 构造超过 ways 的访问 | 最早插入项被替换 | 替换策略由 CacheLevel + replacement policy 决定，UI 仅显示结果 | 待最终手工点击确认 |
| UI-09 | LRU | 先重复访问一项再触发替换 | 最久未使用项被替换 | 替换策略由 CacheLevel + replacement policy 决定，UI 仅显示结果 | 待最终手工点击确认 |
| UI-10 | Step | 连续点击 Step | 每次只前进一条访问 | Step 逻辑已按单次 simulator.Access() 执行，并更新 nextIndex 与状态 | 已在代码层确认 |
| UI-11 | Run All | 点击 Run All | 执行剩余访问，不重复已执行项 | Run All 循环执行剩余访问并重置 last result/trace index | 已在代码层确认 |
| UI-12 | Reset | 执行若干步后 Reset | line、统计和步骤位置清空 | ResetSession() 会清空 simulator、nextIndex 与状态，并刷新表格 | 已在代码层确认 |
| UI-13 | 空 Trace | 清空输入后运行 | 给出提示，不崩溃 | 空 trace 会在 Step/Run All/Reset 中被检测并弹出错误提示 | 已在代码层确认 |
| UI-14 | 错误 Trace | 输入 `R xyz` | 提示准确行号 | MemoryTraceParser 会抛出带行号的 invalid_argument，UI 通过 MessageBox 展示 | 已在代码层确认 |
| UI-15 | 文件导入 | 导入正常 trace 文件 | 内容正确加载并可执行 | 实现了 CFileDialog + ParseFile/ParseText 读取逻辑，并写回编辑框 | 待最终手工点击确认 |
| UI-16 | L1/L2 刷新 | 执行 hit、miss、eviction | 两级网格与核心状态一致 | UI 读取 L1/L2 Sets() 并按 AccessResult 高亮 | 待最终手工点击确认 |
| UI-17 | 高亮 | 执行单步 | 当前命中/填充/替换清楚可见 | 状态列已显示 L1 Hit/L2 Hit/New/Replaced，并根据 set/line 高亮该行 | 待最终手工点击确认 |
| UI-18 | 窗口布局 | 默认分辨率下查看全部区域 | 关键按钮和表格不遮挡 | 布局已扩展为配置区 + Trace 区 + Control 区 + Statistics 区 + L1/L2 Cache 区 | 待最终手工窗口缩放确认 |

测试环境：

```text
测试人：
日期：
Windows 版本：
Visual Studio 配置：Debug/Release + x64/Win32
commit：
遗留问题：
```
