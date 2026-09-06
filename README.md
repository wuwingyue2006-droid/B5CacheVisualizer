# B5 多级 Cache 映射与替换算法可视化演示系统

本项目使用 C++17 和 MFC 实现两级 Cache 教学模拟器。程序支持配置 L1、L2 Cache，在同一条 Memory Trace 下展示映射、替换、命中、缺失、填充和驱逐过程，并提供局部性 Trace 生成、策略对比和实验结果导出。

## 当前状态

- 阶段 03 至阶段 07 功能已经集成到同一代码基线；
- Debug x64 和 Release x64 均纳入统一构建脚本；
- 自动化测试共 90 项，覆盖核心算法、Trace、策略对比、导出和稳定性边界；
- Release 使用静态 MFC 和静态 C++ 运行库，便于在未安装 Visual Studio 的电脑上直接运行；
- 本机 Release 验收记录见 `docs/tasks/08-release-preparation.md`；
- 跨电脑结果由组员填写 `docs/testing/CROSS_PC_TEST_CHECKLIST.md`。

## 主要功能

### 基础模拟

- L1 和 L2 的 Cache Size、Block Size、Associativity 配置；
- Direct Mapping、Fully Associative、Set Associative；
- FIFO 和 LRU；
- 十进制、十六进制、R/W、注释和空白行 Trace；
- Step、Run All、Reset。

### 教学可视化

- L1、L2 Cache Line 网格；
- Hit、New、Replaced、Dirty 状态高亮；
- Tag、Set、Offset 地址拆分；
- CPU、L1、L2、Memory 访问路径；
- 上一步、下一步、自动播放、暂停、停止和速度切换；
- 命中结果组成图和累计命中率趋势图。

### 功能拓展

- Sequential、Loop、Random、Hot Set、Mixed R/W Trace 生成器；
- 固定随机种子的可复现实验；
- Direct、Set、Fully 多方案策略对比；
- 教学预设和冲突缺失 Trace；
- 单实验与对比结果 CSV/TXT 导出。

## 直接运行

最终提交包中的程序位于：

```text
Release/B5CacheVisualizer.exe
```

双击即可启动。建议屏幕分辨率不低于 1366 x 768。

快速演示：

1. 启动程序并保留默认配置；
2. 载入 `examples/demo-complete.trace`；
3. 使用 Next 观察 Memory Miss、L1 Hit、L2 Hit 和 Dirty/eviction；
4. 使用 Auto、Pause 和速度选项演示播放控制；
5. 打开 Compare，选择 Teaching presets 和 conflict-miss teaching trace；
6. 运行三种方案并观察命中率差异；
7. 导出一次单实验结果和一次对比结果。

## 从源码构建

环境要求：

- Windows 10 或 Windows 11；
- Visual Studio 2022；
- 使用 C++ 的桌面开发工作负载；
- MFC/ATL 组件。

打开 `B5CacheVisualizer.sln`，选择 `Release | x64` 构建。也可以运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Configuration Release -Platform x64
```

## 测试

Release x64 完整测试：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\test.ps1 -Configuration Release -Platform x64
```

期望结果：

```text
90/90 tests passed.
```

## 生成最终提交包

```powershell
powershell -ExecutionPolicy Bypass -File scripts\package-release.ps1
```

脚本会先执行 Release x64 构建和测试，再在 `dist` 中生成：

```text
B5CacheVisualizer-submit/
B5CacheVisualizer-submit.zip
```

ZIP 包含源码、Release exe、示例 Trace、测试与交付说明，不包含 `.git`、`.vs`、`obj`、PDB 和其他中间文件。

## 示例 Trace

- `examples/demo-complete.trace`：基础完整演示；
- `examples/conflict-miss.trace`：直接映射冲突缺失；
- `examples/locality-loop.trace`：时间局部性循环访问；
- `examples/mixed-read-write.trace`：读写混合与 Dirty 状态。

## Release 截图

- `docs/screenshots/release-main.png`：默认 Trace 完整执行后的主界面；
- `docs/screenshots/release-comparison.png`：三组教学预设的策略对比结果。

## 目录

```text
B5CacheVisualizer/    MFC 界面和展示控制器
B5CacheCoreTests/     90 项自动化测试
src/                  核心、映射、替换、统计、Trace、对比和导出
examples/             可直接导入的演示 Trace
docs/                 架构、接口、任务卡和验收记录
scripts/              构建、测试、工作区检查和打包脚本
```

## 模型边界

本程序面向 Cache 原理教学，采用单线程、两级 Cache 和合成 Memory Trace。它不模拟真实 CPU 的流水线、TLB、硬件预取、多核一致性和精确访存延迟，因此程序输出用于比较 Cache 配置与访问局部性，不等同于真实处理器性能。

## 团队协作

- 功能分支通过 Pull Request 合入 `dev`；
- 稳定阶段版本由组长从 `dev` 合入 `main`；
- 不直接在 `main` 或 `dev` 上开发；
- 公共接口变化必须同步修改代码、文档和测试。
