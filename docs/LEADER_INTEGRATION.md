# B5 组长集成手册

## 1. 组长负责什么

普通成员只负责把经过测试的代码 push 到自己的 feature 分支。组长负责：

- 确认成员分支来自最新 `dev`；
- 检查 Pull Request 的目标是 `dev`；
- 检查文件是否越界；
- 检查公共接口变化；
- 检查测试结果；
- 决定是否合并；
- 每次合并后验证新版 `dev`；
- 最后把稳定 `dev` 合并到 `main`。

组长不要通过微信接收 `.cpp` 文件后手工复制到项目中。

## 2. 骨架合并后的统一通知

本项目骨架 PR 合并到 `dev` 后，在群里发送：

```text
【B5 正式分工开始】

项目骨架已经合并到 dev。所有成员先进入自己的本地仓库，然后执行：

git switch dev
git pull --ff-only
git status

确认工作区干净后，按照个人任务卡创建且只创建自己的分支。

A：feature-core
B：feature-mapping
C：feature-replacement
D：feature-ui
E：feature-statistics

每个人必须先阅读 AGENTS.md、ARCHITECTURE.md、INTERFACES.md、OWNERSHIP.md 和自己的任务卡。没有分配给自己的文件不要修改。
```

## 3. 成员阶段完成后的报告

成员必须提交：

```text
【B5 阶段功能上传】
成员代号：
个人分支：
本次功能：
主要修改文件：
测试命令：
测试结果：
公共接口变化：无/有
commit：
push 是否成功：是
仍存在问题：无/请说明
```

报告不完整时先让成员补充，不立即合并。

## 4. 创建或检查 Pull Request

正确方向：

```text
base: dev ← compare: feature-成员模块
```

错误方向：

```text
base: main
base: feature-其他成员
compare: dev
```

PR 标题示例：

```text
Implement associative mapping strategies
Implement LRU replacement policy
Complete cache statistics and trace parsing
Complete two-level cache simulation flow
Add interactive cache visualization UI
```

## 5. 逐项审核

### 5.1 文件范围

- B 是否只改 mapping 和 MappingTests；
- C 是否只改 replacement 和 ReplacementTests；
- E 是否只改 statistics、trace 和对应测试；
- A 是否主要只改 core 和 CoreTests；
- D 是否主要只改 MFC UI 和 UI 测试清单；
- 是否混入 `.vs`、`bin`、`obj`、`x64`、exe、pdb；
- 是否意外格式化大量无关文件。

### 5.2 公共接口

如果 PR 修改了 `CacheTypes.h`、公共头文件或 `INTERFACES.md`：

1. 找到成员的公共接口修改申请；
2. 确认所有受影响成员已经知道；
3. 确认文档和测试同步修改；
4. 不满足时要求修改，不直接合并。

### 5.3 功能和测试

- PR 描述是否说明完成内容；
- 对应模块测试是否增加；
- `scripts/test.ps1` 是否通过；
- TODO 是否正确移除；
- 是否把未实现内容伪装为正常结果；
- D 是否填写 UI 人工测试清单。

## 6. 建议合并顺序

```text
feature-mapping
→ feature-replacement
→ feature-statistics
→ feature-core
→ feature-ui
```

如果某成员尚未完成，可以等待；不要为追求顺序而合并未测试代码。

## 7. 每次合并后验证 dev

在组长电脑的仓库根目录运行：

```powershell
git switch dev
git pull --ff-only
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
git status
```

正确结果：

- 编译 0 error；
- 所有测试通过；
- `git status` 工作区干净。

如果合并后测试失败：

- 不继续合并下一位；
- 保留完整错误；
- 在对应 PR 中说明；
- 优先让原成员在其 feature 分支修复并更新 PR；
- 不使用 `git reset --hard` 或 force push 修改共享历史。

## 8. 通知其他成员更新

每形成一个可用新版 `dev`，群里发送：

```text
【dev 已更新】
本次合入模块：
当前 dev 测试结果：全部通过
受影响接口：无/请说明

所有成员在开始下一次开发前执行：
git status
git switch dev
git pull --ff-only
git switch 自己的feature分支
git merge dev

出现冲突立即停止并联系组长。
```

## 9. dev 合并到 main

只有满足以下条件才进行：

- 五个模块都已合入；
- Debug x64 编译通过；
- Release x64 编译通过；
- 核心测试全部通过；
- UI 人工测试完成；
- 没有未处理的严重问题；
- README 与最终功能一致；
- 组内确认该版本可提交或演示。

通过 Pull Request：

```text
base: main ← compare: dev
```

PR 标题建议：

```text
Release B5CacheVisualizer v1.0
```

合并后建议创建版本标签，例如 `v1.0.0`。标签和 Release 在最终版本确认后再做，不提前创建“final2”“最终版3”等临时版本。

## 10. 可直接交给组长 AI 的审核提示词

```text
你是 B5CacheVisualizer 的组长集成助手。请先读取 AGENTS.md、docs/ARCHITECTURE.md、docs/INTERFACES.md、docs/OWNERSHIP.md、docs/LEADER_INTEGRATION.md 和本 PR 对应的成员任务卡。

本轮只审核一个 feature 分支到 dev 的 Pull Request。先确认 base=dev、compare=正确的 feature 分支，然后检查文件范围、公共接口变化、TODO、测试覆盖和生成文件。运行 scripts/test.ps1。发现问题时给出具体文件和修改要求，不自行把失败代码合入。全部通过后才建议组长在 GitHub 页面合并。不要直接改 main，不使用 force/reset/clean。
```
