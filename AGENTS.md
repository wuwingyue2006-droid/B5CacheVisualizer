# B5CacheVisualizer AI 协作规则

本文件适用于在本仓库中工作的所有 AI 助手。开始任何修改前必须先阅读本文件、`docs/ARCHITECTURE.md`、`docs/INTERFACES.md` 和对应成员任务卡。

## 开始操作前

先检查并向成员说明以下结果：

```powershell
Get-Location
git status
git remote -v
git branch --show-current
```

- 远程仓库必须是 `wuwingyue2006-droid/B5CacheVisualizer`。
- 成员开发时必须位于组长指定的 `feature-*` 分支。
- 如果当前是 `main` 或 `dev`，停止修改并帮助成员切换到其个人分支。
- 如果存在不明的未提交修改，先解释并征求成员确认，不能覆盖或删除。

## 修改范围

- 只修改成员任务卡允许的文件。
- `src/common/CacheTypes.h`、`docs/INTERFACES.md`、解决方案和项目文件属于公共区域。
- 修改公共区域前必须先向组长报告修改理由、旧接口、新接口和受影响模块。
- UI 事件处理只负责收集输入、调用核心接口和显示结果；不得把映射、替换或统计算法写入对话框类。
- 不要大规模重命名、移动或格式化其他成员负责的文件。

## Git 安全要求

- 不直接向 `main` 或 `dev` commit/push。
- 不使用 `git push --force`、`git reset --hard`、`git clean -fd`、rebase 或 cherry-pick。
- 不删除 `.git`，不因普通报错重新 clone。
- 出现冲突时立即停止，保留 `git status` 和冲突输出，交给组长处理。
- 提交前依次检查 `git branch --show-current`、`git status` 和 `git diff --staged`。

## 质量要求

- C++ 代码使用 C++17。
- 公共 API 位于 `b5cache` 命名空间。
- 新功能必须有对应的核心测试或明确的人工 UI 测试步骤。
- 交付前运行 `powershell -ExecutionPolicy Bypass -File scripts/test.ps1`。
- 在 Linux Cloud Agent 上没有 Visual Studio 和 MFC。核心测试与界面分离，改运行 `bash scripts/test-core.sh`（系统 g++，C++17）。MFC 对话框仍只在 Windows + Visual Studio 2022 上构建。
- 不提交 `.vs`、`Debug`、`Release`、`x64`、可执行文件或用户配置。
- 未实现功能必须使用明确的 `TODO(A/B/C/D/E)` 标记，不能伪装成已完成。

## 完成汇报

AI 最终应告诉成员：修改了哪些文件、实现了什么、如何测试、测试结果、是否修改公共接口，以及建议使用的 commit 信息。AI 不得自行把个人分支合并到 `dev` 或 `main`。

## Cloud Agent（Linux）

界面程序是 MFC 对话框，只能在 Windows 上用 Visual Studio 2022 打开 `B5CacheVisualizer.sln` 构建。Cloud Agent 使用 Ubuntu，镜像里已有 g++、clang++ 和 CMake。核心模拟器与 `B5CacheCoreTests` 不包含 MFC 头文件，用下面的命令编译并运行全部核心测试：

```bash
bash scripts/test-core.sh
```

该脚本对应 Windows 上 `scripts/test.ps1` 里的核心测试，输出在 `bin/linux/Debug/B5CacheCoreTests`（已被 `.gitignore` 忽略）。不要在 Cloud Agent 的 install 或 start 里安装 Visual Studio、MSBuild 或 MFC，也不要启动界面程序。
