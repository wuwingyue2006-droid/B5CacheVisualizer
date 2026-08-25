# B5 成员本地 AI 与 Git 协作完整流程

> 适用对象：不熟悉 Git、需要 AI 一步一步指导的 B5 项目成员。
> 适用环境：Windows、PowerShell、Git for Windows、Visual Studio 2022。
> 仓库：`https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git`

## 一、这份指南解决什么问题

本流程不假设成员使用的 AI 能登录、浏览或直接操作 GitHub。

成员的 AI 只需要具备下面任意一种能力即可：

- 能读取成员电脑上的本地项目文件并运行 PowerShell；或
- 不能直接运行命令，但能根据成员复制回来的终端输出继续指导。

GitHub 网页上的操作由成员本人完成：

- 接受 Collaborator 邀请；
- 完成浏览器登录或授权；
- 查看远程分支；
- 创建 Pull Request；
- 把 PR 链接发给组长。

AI 不需要 GitHub 插件，也不需要获得成员的密码、令牌或浏览器控制权。

## 二、人、AI 和组长分别负责什么

| 角色 | 负责内容 | 不负责内容 |
|---|---|---|
| 成员本人 | 接受邀请、浏览器登录、确认弹窗、把命令输出交给 AI、检查后确认提交 | 不自行解决复杂冲突，不猜测危险命令 |
| 成员的本地 AI | 读取本地规范和任务卡、检查分支、修改允许范围内的代码、编译测试、根据终端输出指导 Git | 不要求能访问 GitHub 网页，不索要密码或令牌，不合并 `dev/main` |
| 组长 | 分配 A–E、确认公共接口、检查 PR、处理冲突、按顺序合入 `dev`、最终发布到 `main` | 不替成员长期维护个人分支 |

如果 AI 说“我无法访问 GitHub”，这不是故障。让 AI 继续处理本地仓库，网页步骤由成员本人完成。

## 三、五位成员固定对应关系

| 代号 | 模块 | 个人分支 | 任务卡 |
|---|---|---|---|
| A | Core / L1-L2 集成 | `feature-core` | `docs/tasks/A-core.md` |
| B | 地址映射 | `feature-mapping` | `docs/tasks/B-mapping.md` |
| C | FIFO/LRU 替换 | `feature-replacement` | `docs/tasks/C-replacement.md` |
| D | MFC UI 与可视化 | `feature-ui` | `docs/tasks/D-ui.md` |
| E | Statistics 与 Trace | `feature-statistics` | `docs/tasks/E-statistics.md` |

成员不能自己更换分支名，也不能同时创建多个 `feature-*` 分支。

## 四、全组必须遵守的安全规则

1. 不在 `main` 上开发。
2. 不在 `dev` 上开发。
3. 只在组长分配的个人 `feature-*` 分支上修改代码。
4. 不把未经测试的代码交给组长合并。
5. 修改公共接口、公共头文件或项目文件前先联系组长。
6. 命令报错或出现 `CONFLICT` 时立即停止。
7. 不把密码、访问令牌、验证码或私钥交给 AI 或发到群里。
8. 不让 AI 为了方便而重写、移动或格式化整个仓库。

普通成员和其 AI 禁止自行执行：

```text
git push --force
git reset --hard
git clean -fd
git rebase
git cherry-pick
```

也不要删除 `.git` 文件夹，不要因为一次报错就重新 clone。

## 五、组长发给每位成员的开始信息

组长必须先填写并发送：

```text
【B5 个人开始信息】
成员代号：A / B / C / D / E
负责模块：
个人分支：feature-________
对应任务卡：docs/tasks/________.md
仓库地址：https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git
允许创建个人分支：是 / 否
允许开始写代码：是 / 否
```

如果“允许创建个人分支”不是“是”，成员只完成首次接入并停留在 `dev`。

如果代号、分支或任务卡不完整，成员和 AI 都不能自行猜测。

## 六、第一次接入仓库

第一次接入的目标只有一个：让本地电脑拥有最新 `dev`，然后停止。

正确顺序：

```text
安装 Git
  ↓
配置本人身份
  ↓
本人接受 GitHub 邀请
  ↓
clone 仓库
  ↓
核对 origin
  ↓
切换并更新 dev
  ↓
停止并向组长报告
```

### 6.1 检查 Git

打开 PowerShell：

```powershell
git --version
```

正常结果类似：

```text
git version 2.xx.x.windows.x
```

如果提示找不到 `git`，先安装 Git for Windows，安装完成后关闭并重新打开 PowerShell。

### 6.2 配置本人 Git 身份

每个人都填写自己的名字和邮箱，不能照抄组长或其他成员：

```powershell
git config --global user.name "你的名字"
git config --global user.email "你的邮箱"
```

检查：

```powershell
git config --global user.name
git config --global user.email
```

这里的邮箱用于标记 commit 作者，不要填写别人的邮箱。

### 6.3 本人接受 Collaborator 邀请

这一步由成员本人在 GitHub 网页完成，AI 不需要进入 GitHub：

1. 登录收到邀请的 GitHub 账号；
2. 打开 GitHub 通知或邀请链接；
3. 确认仓库为 `wuwingyue2006-droid/B5CacheVisualizer`；
4. 点击接受邀请；
5. 不要把登录密码或令牌复制给 AI。

仓库是 Public 时可能可以直接 clone，但没有接受邀请通常不能 push 个人分支。

### 6.4 选择本地父目录

示例父目录：

```powershell
cd D:\Projects
```

先检查当前位置：

```powershell
Get-Location
Get-ChildItem -Force
```

如果这里已经存在同名 `B5CacheVisualizer` 文件夹，不要覆盖，也不要立刻重新 clone。把结果交给 AI 判断。

### 6.5 clone 仓库

确认父目录中没有冲突的同名文件夹后运行：

```powershell
git clone https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git
```

然后进入仓库：

```powershell
cd B5CacheVisualizer
```

### 6.6 核对仓库身份

依次运行：

```powershell
Get-Location
git status
git remote -v
git branch -a
```

必须确认：

- 当前路径最后一级是 `B5CacheVisualizer`；
- `git status` 没有提示“不是 Git 仓库”；
- `origin` 指向组长的仓库；
- 能看到 `remotes/origin/dev`。

如果 `origin` 指向其他人的仓库或其他项目，立即停止。

### 6.7 切换并更新 dev

先尝试：

```powershell
git switch dev
```

如果提示本地没有 `dev`，但 `git branch -a` 能看到 `remotes/origin/dev`，运行：

```powershell
git switch --track origin/dev
```

再下载最新内容：

```powershell
git pull --ff-only
```

最后检查：

```powershell
git branch --show-current
git status
```

正常情况必须是：

```text
当前分支：dev
nothing to commit, working tree clean
```

此时停止，不修改代码，不创建个人分支，等待组长发出“允许开始”。

## 七、允许开始后：创建唯一的个人分支

以下示例使用成员 B 的 `feature-mapping`。其他成员必须替换为组长指定的准确分支。

### 7.1 先确认本地没有遗留修改

```powershell
git status
```

只有出现 `working tree clean` 才继续。

### 7.2 从最新 dev 创建分支

```powershell
git switch dev
git pull --ff-only
git switch -c feature-mapping
```

一次只执行一条。如果任何一步报错，不执行下一条。

### 7.3 第一次上传个人分支

```powershell
git push -u origin feature-mapping
```

这里可能出现浏览器授权。该步骤由成员本人完成：

1. 确认浏览器登录的是收到邀请的 GitHub 账号；
2. 按提示允许 Git Credential Manager；
3. 返回 PowerShell 等待命令结束；
4. 不把授权码、密码或令牌交给 AI。

AI 只能根据 `git push` 的最终输出判断是否成功，不能在没有证据时声称远程分支已经创建。

### 7.4 向组长报告

运行：

```powershell
git branch --show-current
git status
```

报告模板：

```text
【B5 个人分支创建报告】
成员代号：
个人分支：
git branch --show-current：
git push 是否成功：
git status 是否干净：
遇到的问题：无 / 请说明
```

## 八、每次让本地 AI 开始写代码前

成员先告诉 AI 自己的代号、分支和本地路径。AI 必须读取本地文件，不依赖 GitHub 网页：

```text
AGENTS.md
docs/ARCHITECTURE.md
docs/INTERFACES.md
docs/OWNERSHIP.md
对应的 docs/tasks/*.md
```

AI 开始修改前必须检查：

```powershell
Get-Location
git status
git remote -v
git branch --show-current
```

只有同时满足下面条件才允许修改：

- 路径是正确仓库；
- `origin` 正确；
- 当前分支是该成员唯一的 `feature-*` 分支；
- 工作区状态已经解释清楚；
- AI 已说明允许修改的文件范围；
- AI 已说明本轮完成标准。

如果当前是 `main` 或 `dev`，AI 必须停止修改并帮助成员切换回个人分支。

## 九、每天开始开发的固定流程

### 9.1 检查上次是否遗留修改

```powershell
git status
```

如果不干净，不要切分支。先让 AI 解释每个修改属于什么。

### 9.2 更新团队基线

工作区干净时：

```powershell
git switch dev
git pull --ff-only
```

### 9.3 回到个人分支并吸收最新 dev

以成员 B 为例：

```powershell
git switch feature-mapping
git merge dev
```

没有冲突才能开始写代码。

如果出现 `CONFLICT` 或 `Automatic merge failed`：

1. 立即停止；
2. 不要 commit；
3. 不要 push；
4. 不要点击编辑器中的“全部接受”；
5. 运行 `git status`；
6. 把完整命令、完整输出和冲突文件列表发给组长。

每天开发前可以记成：

```text
status → dev → pull → 个人 feature → merge dev → 开发
```

## 十、AI 本地开发流程

AI 应把任务拆成小步骤：

1. 读取任务卡和相关代码；
2. 说明本轮只准备实现哪一个小功能；
3. 只修改任务卡允许的文件；
4. 编译或运行本模块测试；
5. 检查失败原因；
6. 修复后再次测试；
7. 向成员说明修改文件和测试结果；
8. 等成员确认后再进入 Git 提交阶段。

AI 不得因为“接口不方便”而直接修改：

```text
src/common/CacheTypes.h
docs/INTERFACES.md
B5CacheVisualizer.sln
*.vcxproj
其他成员负责的源文件和测试文件
```

确实需要公共接口变化时，AI 只能先生成：

```text
【公共接口修改申请】
提出成员：
修改原因：
原接口：
建议新接口：
影响模块：A/B/C/D/E
需要同步修改的文件：
兼容方案：
```

获得组长同意后才能实施。

## 十一、编译和测试

优先在仓库根目录运行统一脚本：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\test.ps1
```

只构建时：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
```

检查是否混入生成文件：

```powershell
powershell -ExecutionPolicy Bypass -File scripts\check-workspace.ps1
```

如果成员的电脑缺少 Visual Studio、MFC 或构建组件，AI 应明确报告环境缺失，不能把“没有运行测试”写成“测试通过”。

## 十二、完成一个小功能后的提交和上传

### 12.1 确认仍在个人分支

```powershell
git branch --show-current
```

必须显示本人的 `feature-*`。显示 `main` 或 `dev` 时立即停止。

### 12.2 查看修改

```powershell
git status
git diff
```

让 AI 检查：

- 文件是否全部属于本人的任务范围；
- 是否混入 `.vs`、`Debug`、`Release`、`x64`、可执行文件；
- 是否意外修改公共文件；
- 是否有调试打印或临时文件。

### 12.3 添加本次功能文件

优先逐个添加经过确认的文件：

```powershell
git add 文件1 文件2
```

只有 AI 和成员已经确认当前全部修改属于同一个功能时，才可以：

```powershell
git add .
```

### 12.4 检查暂存内容

```powershell
git status
git diff --staged
```

这一步不能省略。发现不认识的文件时停止，不提交。

### 12.5 提交

让 AI 根据实际功能生成清楚的英文 commit 信息。例如：

```powershell
git commit -m "Implement associative mapping strategies"
```

不要使用：

```text
update
修改
111
final
最终版2
```

### 12.6 上传个人分支

```powershell
git push
```

如果浏览器要求登录或授权，由成员本人完成。AI 不接收任何凭据。

完成后向组长发送：

```text
【B5 阶段功能上传】
成员代号：
个人分支：
本次完成功能：
主要修改文件：
测试命令：
测试结果：
commit 信息：
git push 是否成功：
公共接口变化：无 / 请说明
仍存在的问题：无 / 请说明
```

每天开发后可以记成：

```text
个人 feature → status/diff → test → add → staged diff → commit → push → 通知组长
```

## 十三、Pull Request 由人操作网页，AI 负责准备文字

个人功能完成阶段目标后，需要通过 PR 进入 `dev`。

AI 即使不能访问 GitHub，也可以根据本地内容准备：

- PR 标题；
- 完成内容；
- 主要修改文件；
- 测试命令和结果；
- 公共接口影响；
- 已知问题。

成员本人打开 GitHub：

1. 进入仓库的 `Pull requests`；
2. 点击 `New pull request`；
3. `base` 选择 `dev`；
4. `compare` 选择自己的 `feature-*`；
5. 查看 `Files changed`，确认没有无关文件；
6. 填入 AI 准备的标题和说明；
7. 点击 `Create pull request`；
8. 把 PR 链接发给组长；
9. 不点击合入 `main`；
10. 普通成员不自行处理复杂冲突。

创建前必须看到：

```text
base: dev ← compare: feature-你的模块
```

PR 说明模板：

```text
## 完成内容

- （填写）

## 主要修改文件

- （填写）

## 测试

- 命令：
- 结果：

## 公共接口影响

- 无 / 请说明

## 仍存在的问题

- 无 / 请说明
```

是否合并、何时合并以及冲突处理由组长决定。

## 十四、PR 合并后的个人分支处理

组长通知 PR 已经合入 `dev` 后，成员开始下一阶段前仍按固定流程：

```powershell
git status
git switch dev
git pull --ff-only
git switch 你的个人feature分支
git merge dev
```

在课程项目结束前不要自行删除远程分支。是否删除由组长统一决定。

## 十五、常见问题

### 15.1 AI 无法访问 GitHub

正常现象。AI 继续读取本地文件、运行本地 Git 命令和准备 PR 文字；成员本人负责网页。

### 15.2 `git` 无法识别

安装 Git for Windows，关闭并重新打开 PowerShell，再运行 `git --version`。

### 15.3 clone 提示目标文件夹已存在

不要覆盖。运行：

```powershell
Get-Location
Get-ChildItem -Force
```

把输出交给 AI 或组长判断。

### 15.4 找不到 dev

```powershell
git fetch origin
git branch -a
```

看到 `remotes/origin/dev` 后：

```powershell
git switch --track origin/dev
```

### 15.5 push 出现 403 或 permission denied

依次确认：

1. 是否接受 Collaborator 邀请；
2. 浏览器是否登录收到邀请的账号；
3. `git remote -v` 是否指向正确仓库；
4. Git Credential Manager 是否完成授权。

不要使用 `--force`，不要把令牌发给 AI。

### 15.6 切换分支提示会覆盖本地修改

立即停止：

```powershell
git status
git diff
```

把输出交给 AI 和组长，不要强制切换。

### 15.7 merge 出现冲突

立即停止并保留现场：

```powershell
git status
```

不要随意选择“保留我的全部”或“保留对方全部”。组长判断后再继续。

### 15.8 AI 无法直接运行命令

让 AI 每次只给一个 PowerShell 命令。成员复制执行并把完整输出贴回，AI 再给下一步。

### 15.9 不确定当前仓库和分支

```powershell
Get-Location
git status
git remote -v
git branch --show-current
```

把四项完整输出交给 AI，先确认环境再继续。

## 十六、可直接复制给 AI 的提示词

### 16.1 第一次接入提示词

```text
你是我的 B5 项目本地 Git 接入助手。我不熟悉 Git，你不需要也不应假设自己能访问 GitHub 网页。

仓库地址：
https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git

本轮唯一目标：
1. 检查 Git；
2. 配置我本人的 Git 姓名和邮箱；
3. 让我本人确认已接受 Collaborator 邀请；
4. 在我指定的父目录 clone 仓库；
5. 核对 origin；
6. 切换并更新 dev；
7. 确认工作区干净后停止。

严格限制：
- 不创建 feature 分支；
- 不修改代码；
- 不 commit 或 push；
- 不执行 git init；
- 不覆盖已有目录；
- 不索要、显示或保存密码和访问令牌；
- 不使用任何强制或清理命令。

每次只给我一个 PowerShell 命令，先解释用途，等我贴回完整输出再继续。出现异常立即停止诊断。现在先问我计划使用的姓名、邮箱和本地父目录，然后从 git --version 开始。
```

### 16.2 每天开始开发提示词

```text
你是我的 B5 项目本地开发助手。你不需要访问 GitHub 网页，只根据本地仓库和终端输出工作。

成员代号：【A/B/C/D/E】
本地仓库路径：【填写】
个人分支：【填写 feature-*】
对应任务卡：【填写 docs/tasks/*.md】

先完整读取本地：
AGENTS.md
docs/ARCHITECTURE.md
docs/INTERFACES.md
docs/OWNERSHIP.md
我的任务卡

然后运行或让我运行：
Get-Location
git status
git remote -v
git branch --show-current

先确认工作区干净，再一步一步执行：切换 dev、git pull --ff-only、切回我的个人分支、git merge dev。出现冲突立即停止。环境确认后，先复述我允许修改的文件、本轮小目标和完成标准，再开始写代码。
```

### 16.3 完成功能后提交提示词

```text
请指导我检查、测试、提交并上传本次 B5 小功能。你不需要访问 GitHub 网页。

先确认：
1. 当前是我的 feature-* 分支；
2. 修改只在任务卡允许范围；
3. 没有生成文件和公共接口意外变化；
4. scripts/test.ps1 已运行并如实记录结果。

然后一条命令一步地指导：git status、git diff、按文件 git add、git diff --staged、git commit、git push。根据实际修改生成清楚的 commit 信息和阶段功能上传报告。不要合并 dev/main，不使用危险命令，不索要任何凭据。
```

### 16.4 出现报错时的提示词

```text
我在 B5 项目执行 Git 或构建命令时遇到错误。请只诊断，不要立即执行破坏性修复。

请先让我提供：
Get-Location
git status
git remote -v
git branch --show-current
刚才执行的完整命令
完整错误输出

不要建议 git reset --hard、git clean -fd、force push、rebase、删除 .git 或重新 clone。若涉及 merge conflict、公共接口、远程权限或可能丢失文件，请明确让我停止并联系组长。
```

## 十七、组长给成员发送的最小文件包

每位成员至少收到：

1. 本文档 `docs/B5成员本地AI与Git协作完整流程.md`；
2. 该成员自己的 `docs/tasks/*.md`；
3. 仓库地址；
4. 成员代号和准确分支名；
5. “允许创建个人分支”和“允许开始写代码”的明确通知。

AI 可以从 clone 后的本地仓库继续读取 `AGENTS.md`、架构、接口和归属文件，因此不需要把整个仓库中的说明逐个复制到聊天中。

## 十八、组长接入验收清单

每位成员开始写代码前，组长确认：

- [ ] Collaborator 邀请已由成员本人接受；
- [ ] Git 已安装；
- [ ] Git 作者姓名和邮箱是成员自己的；
- [ ] clone 地址正确；
- [ ] 本地 `dev` 已更新；
- [ ] 个人分支与 A–E 分工一致；
- [ ] 第一次 push 成功；
- [ ] 当前工作区干净；
- [ ] 成员知道 AI 不需要访问 GitHub；
- [ ] 成员不会把密码或令牌交给 AI；
- [ ] 成员知道不能直接修改 `main/dev`；
- [ ] 成员知道冲突和报错时立即停止；
- [ ] 成员已经把自己的任务卡交给本地 AI；
- [ ] AI 已复述允许修改文件和完成标准。

全部通过后，成员才能正式开发。

## 十九、一页速记

第一次接入：

```text
本人接受邀请 → 安装/配置 Git → clone → dev → pull → 停止
```

第一次开始：

```text
最新 dev → 创建唯一 feature → push → 报告组长
```

每天开发前：

```text
status → dev → pull → 个人 feature → merge dev
```

每天开发后：

```text
test → status/diff → add → staged diff → commit → push → 报告组长
```

阶段完成：

```text
AI 准备 PR 文字 → 本人创建 feature → dev 的 PR → 组长检查和合并
```

任何异常：

```text
立即停止 → 保留完整命令和输出 → 交给 AI 与组长
```
