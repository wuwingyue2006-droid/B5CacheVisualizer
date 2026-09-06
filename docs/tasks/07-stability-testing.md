# 阶段 07 任务卡：集中稳定性测试

## 阶段信息

```text
顺序：07
分支：feature-07-stability-testing
主题：对基础版和阶段 03～06 扩展进行集中回归、压力与连续操作测试
前置：阶段 06 已合入 dev；本分支从最新 origin/dev 创建
状态：自动化稳定性测试完成，待组长人工 UI 复验
```

本阶段停止新增功能，只执行“测试 → 修 Bug → 再测试”。测试结果必须能够定位到配置、Trace、访问流程、对比、导出或展示控制器中的具体边界。

## 自动化范围

- 覆盖 Direct、Set、Fully 在 FIFO/LRU 下的八组 L1/L2 配置矩阵。
- 集中复核 L1/L2 非法 Size、Block Size、associativity 与映射约束。
- 解析并执行 1000 条和 10000 条混合十进制/十六进制、R/W、注释 Trace。
- 验证大 Trace 下统计恒等式、有限命中率、访问顺序与 L2 跳过规则。
- 验证 Reset 后同一 Trace 的逐条结果和最终统计完全可复现。
- 验证三方案 10000 条 Trace 对比与单独运行一致。
- 验证 10000 条实验导出仍包含末尾记录、最终统计和结论。
- 对纯 C++ `VisualizationController` 执行前后步进、首尾边界、回看保护、播放/暂停/停止、速度和 Reset 测试。

## 允许修改范围

```text
B5CacheCoreTests/StabilityTests.cpp
B5CacheCoreTests/TestSuites.h
B5CacheCoreTests/TestMain.cpp
B5CacheCoreTests/B5CacheCoreTests.vcxproj
B5CacheCoreTests/B5CacheCoreTests.vcxproj.filters
docs/tasks/07-stability-testing.md
docs/testing/UI_TEST_CHECKLIST.md
```

若新增测试暴露真实 Bug，可修改对应实现文件，但必须在本任务卡记录症状、原因、修复和回归测试；不得借机增加新功能或改变公共接口。

## 人工 UI 复验

- Step/Run All 到末尾后重复点击不崩溃、不重复计数。
- 自动播放期间暂停、停止、Reset、修改 Trace/配置时无残留 Timer。
- 错误 Trace 后输入正确 Trace，可以建立全新会话。
- 依次运行八组配置矩阵，统计、图表、地址拆分和 Cache 表同步。
- 1000 条 Trace 可完成；10000 条 Trace 使用 Run All，不要求逐帧人工观察。
- 主窗口、Trace Generator、Comparison 在 1366×768 和 1920×1080 下无新增遮挡。
- 单实验与对比 CSV/TXT 各实际保存一次，检查 UTF-8、末尾 Trace 和统计一致性。

## 完成判定

- Debug x64 完整构建 0 警告、0 错误。
- 全量核心测试通过，新增稳定性测试无随机失败。
- `git diff --check` 通过，工作区不包含构建产物。
- 所有发现的阻断性 Bug 已修复或明确记录。
- 不修改公共接口，不新增阶段范围外功能。

## 建议提交信息

```text
Add stage 7 stability regression coverage
```

## 开发侧验收记录（2026-09-06）

- 从已包含阶段 06 修复提交的 `origin/dev` 创建阶段 07 分支，基础版与阶段 03～06 扩展均处于同一测试基线。
- 新增 15 项稳定性测试，测试总数从 75 增加到 90；覆盖八组配置矩阵、十类非法配置、1000/10000 条 Trace、Reset 重放、三方案压力对比、万条导出和展示控制器连续操作。
- Debug x64 完整构建结果为 0 警告、0 错误。
- 完整测试及随后两次直接复跑均为 90/90 通过，未发现随机失败或需要修改生产代码的缺陷。
- 未修改 Cache 公共接口，也未新增阶段范围外功能；组长仍需按 `docs/testing/UI_TEST_CHECKLIST.md` 的 UI-35～UI-42 完成人工界面复验。
