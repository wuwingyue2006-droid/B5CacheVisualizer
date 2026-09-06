# 阶段 08 任务卡：Release 与程序交付

## 阶段信息

```text
顺序：08
分支：feature-08-release-preparation
主题：把阶段 07 基线整理成可构建、可运行、可测试和可打包的软件交付版本
前置：阶段 07 自动化稳定性测试完成
状态：本机 Release 验收完成，待组员跨电脑复验
```

本阶段只处理程序本体及其交付材料，不新增 Cache 算法，不制作实验报告或答辩幻灯片。

## 允许修改范围

```text
B5CacheVisualizer/B5CacheVisualizer.vcxproj
README.md
.gitignore
examples/*
scripts/package-release.ps1
docs/DELIVERY.md
docs/tasks/08-release-preparation.md
docs/tasks/README.md
docs/testing/CROSS_PC_TEST_CHECKLIST.md
docs/screenshots/*
```

## 交付目标

- Release x64 采用静态 MFC 和静态 C++ 运行库；
- Release x64 完整构建无错误、无警告；
- Release 核心测试 90/90 通过；
- 不打开 Visual Studio 时可以直接启动 Release exe；
- 提供四条可直接导入的演示 Trace；
- 提供可重复执行的精简打包脚本；
- 最终 ZIP 小于 25 MB；
- 跨电脑结果由组员实测后填写，不提前标记通过。

## 本机验收记录

验收日期：2026-09-06。

- Release x64 完整构建成功，结果为 0 警告、0 错误；
- Release 核心测试 90/90 通过；
- Release exe 使用 `/MT`，依赖检查未发现 MFC、VCRUNTIME 或 MSVCP 动态运行库；
- 未打开 Visual Studio，直接启动 `B5CacheVisualizer.exe` 成功；
- Next 后正确显示 Memory Miss、地址拆分、访问路径和两级填充；
- Run All 完成 5/5，统计图、趋势图、Cache 表和 Dirty/eviction 状态同步；
- Trace Generator 窗口可打开并正常关闭；
- Comparison 可载入三组教学预设、运行冲突 Trace 并生成对比图；
- 程序关闭后未发现残留窗口；
- 最终主界面与策略对比截图保存在 `docs/screenshots`；
- 提交包由 `scripts/package-release.ps1` 生成并执行 25 MB 上限检查；
- ZIP 共 91 个条目，大小为 1.56 MB，包含 4 条示例 Trace；
- ZIP 内未发现 `.git`、`.vs`、`obj`、Debug、PDB、日志或用户配置；
- 包内 exe 与已测试 Release exe 的 SHA-256 哈希一致；
- 从 `dist/B5CacheVisualizer-submit/Release` 直接启动包内 exe 成功，正常关闭且无残留窗口。

## 跨电脑状态

详见 `docs/testing/CROSS_PC_TEST_CHECKLIST.md`。当前状态保持为待组员实测。

## 建议提交信息

```text
Prepare stage 8 release delivery
```
