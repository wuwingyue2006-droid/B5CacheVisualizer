# B5 五人任务卡使用说明

## 1. 开始条件

项目骨架已经合并到 `dev`。成员确认自己的 A–E 代号和固定分支名后，只要本地 `dev` 已更新且工作区干净，就可以随时创建并上传自己的唯一 `feature-*` 分支，不需要等待组长另行批准。

每位成员先从最新 `dev` 创建且只创建自己的分支：

| 代号 | 分支 | 任务卡 |
|---|---|---|
| A（技术组长） | `feature-core` | `A-core.md` |
| B | `feature-mapping` | `B-mapping.md` |
| C | `feature-replacement` | `C-replacement.md` |
| D | `feature-ui` | `D-ui.md` |
| E | `feature-statistics` | `E-statistics.md` |

## 2. 给成员发送什么

组长给每位成员发送：

1. 《B5 成员本地 AI 与 Git 协作完整流程》；
2. 《B5 五人精确分工与组长职责》；
3. 该成员自己的任务卡；
4. 仓库地址；
5. 成员代号和准确的个人分支名；
6. 当前阶段任务和完成标准。

成员的 AI 不需要具备 GitHub 插件、网页访问或账号登录能力。GitHub 邀请、浏览器授权和 Pull Request 页面由成员本人操作；AI 只依据本地仓库、Git 命令输出和任务卡进行指导。

成员把自己的任务卡交给本地 AI，要求 AI 先读取仓库根目录 `AGENTS.md`、`docs/ARCHITECTURE.md`、`docs/INTERFACES.md`、`docs/OWNERSHIP.md`、`docs/B5成员本地AI与Git协作完整流程.md` 和该任务卡。

## 3. 第一次创建分支

把下面的 `feature-xxx` 替换为任务卡规定的分支：

```powershell
git switch dev
git pull --ff-only
git status
git switch -c feature-xxx
git push -u origin feature-xxx
```

如果 `git status` 不是干净状态，不继续创建分支。

## 4. 统一完成条件

每位成员都必须满足：

- 只修改任务卡允许的文件；
- TODO 功能已经实现；
- 新增或更新对应测试；
- `scripts/test.ps1` 全部通过；
- 没有编译警告；
- 没有生成文件进入 Git；
- commit 信息有意义；
- 已 push 到自己的 feature 分支；
- 已向组长提交阶段功能上传报告；
- 未自行合并到 `dev/main`。

## 5. 组长建议集成顺序

```text
B Mapping
→ C Replacement
→ E Statistics/Trace
→ A Core
→ D UI
```

每合并一个 PR，都在最新 `dev` 上重新运行完整测试。
