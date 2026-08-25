# B5CacheVisualizer

暑期《计算机综合课程设计》B5：多级 Cache 映射与替换算法可视化演示系统。

## 项目目标

- L1 / L2 Cache；
- Cache Size / Block Size / Associativity 配置；
- Direct Mapping；
- Fully Associative；
- Set Associative；
- FIFO；
- LRU；
- Memory Trace 输入和文件导入；
- Hit / Miss / Hit Rate / Miss Rate；
- Cache Line 状态与替换过程可视化。

## 开发环境

- Windows；
- Visual Studio 2022；
- “使用 C++ 的桌面开发”工作负载；
- MFC/ATL 组件；
- C++17；
- Git。

## 当前框架状态

当前项目骨架包含：

- 可启动的 MFC 对话框程序；
- 与 MFC 分离的核心模拟器；
- L1/L2 Cache 基础访问流程；
- Mapping 和 Replacement 策略接口；
- Statistics 与 Memory Trace 模块；
- Direct Mapping + FIFO 可运行基线；
- 独立的核心自动测试项目；
- A–E 五人文件归属和任务卡。

有意保留的成员任务：

- A：Core 行为完善与集成；
- B：Fully Associative / Set Associative；
- C：LRU；
- D：完整 MFC 配置与可视化；
- E：完整统计、Trace 边界处理和文件导入。

未实现功能使用明确的 `TODO(A/B/C/D/E)` 或异常提示标记。

## 打开和构建

用 Visual Studio 2022 打开：

```text
B5CacheVisualizer.sln
```

推荐配置：

```text
Debug | x64
```

也可以在仓库根目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
```

运行完整核心测试：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

## 目录说明

```text
B5CacheVisualizer/    MFC 应用
B5CacheCoreTests/     核心自动测试
src/common/           公共数据结构
src/core/             两级 Cache 调度
src/mapping/          映射策略
src/replacement/      替换策略
src/statistics/       统计
src/trace/            Trace 解析
docs/                 架构、接口、协作与任务卡
scripts/              固定构建和测试脚本
```

## 必读文档

- [AI 协作规则](AGENTS.md)
- [总体架构](docs/ARCHITECTURE.md)
- [公共接口](docs/INTERFACES.md)
- [文件归属](docs/OWNERSHIP.md)
- [五人任务卡说明](docs/tasks/README.md)
- [第一次加入 GitHub 项目流程](docs/B5成员第一次加入GitHub项目流程.md)
- [组员 Git 一步一步操作卡](docs/B5组员Git一步一步操作卡.md)
- [组长集成手册](docs/LEADER_INTEGRATION.md)

## 分支约定

```text
main
 └── dev
      ├── feature-core
      ├── feature-mapping
      ├── feature-replacement
      ├── feature-ui
      └── feature-statistics
```

- `main`：稳定阶段版本和最终提交版本；
- `dev`：团队集成版本；
- `feature-*`：成员个人开发分支。

任何成员都不要直接在 `main` 或 `dev` 上写功能代码。个人功能通过 Pull Request 合并到 `dev`，全组稳定后再由组长把 `dev` 合并到 `main`。
