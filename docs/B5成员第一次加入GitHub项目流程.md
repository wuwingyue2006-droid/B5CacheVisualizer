# B5 组员第一次加入 GitHub 项目流程

> 适用项目：`B5CacheVisualizer`
> 仓库地址：<https://github.com/wuwingyue2006-droid/B5CacheVisualizer>
> 适用对象：B5 五人小组全体成员
> 当前阶段目标：所有成员完成仓库接入并停在 `dev` 分支，等待正式分工。

## 一、先看最重要的结论

Git 协作环境搭好之后，下一件事才是正式确定五人分工。全组统一按下面的顺序推进：

```text
接受仓库邀请
    ↓
安装并配置 Git
    ↓
把仓库 clone 到自己的电脑
    ↓
切换到 dev 并拉取最新内容
    ↓
停止操作，向组长报告接入结果
    ↓
全组确定模块、接口和验收标准
    ↓
再从 dev 创建各自的 feature-* 分支
```

现在不要自己随便创建 `feature-*` 分支。模块名称、公共接口和文件结构还没有最终确定，提前创建分支容易造成五个人的代码互不兼容。

## 二、我们的分支结构

目前计划采用下面的分支结构，最终名称以组长发布的分工通知为准：

```text
main
 └── dev
      ├── feature-core
      ├── feature-mapping
      ├── feature-replacement
      ├── feature-ui
      └── feature-statistics
```

- `main`：最稳定、最终可提交的版本。
- `dev`：全组代码的日常集成版本。
- `feature-*`：成员完成各自模块的开发分支。

必须记住：

1. 平时不要直接在 `main` 上开发。
2. 个人代码先进入自己的 `feature-*` 分支。
3. 一个阶段完成并经过自测后，通过 Pull Request 合并到 `dev`。
4. 整个程序集成、测试并确认稳定后，才把 `dev` 合并到 `main`。

## 三、正式操作前的准备

每位成员需要准备：

- 一个自己的 GitHub 账号；
- Windows 电脑；
- Git for Windows；
- 可以正常使用的 PowerShell；
- 稍后用于开发的 Visual Studio 2022；
- 一处固定的项目存放位置，例如 `D:\Projects`。

注意：

- 不要用“Download ZIP”代替 `git clone`。ZIP 文件没有完整的 Git 协作信息，不能正常拉取和提交代码。
- 不要把仓库放在会频繁自动同步、改名或清理文件的目录中。
- 路径尽量简短，建议使用英文目录，避免后续构建工具出现路径问题。
- 每个人必须使用自己的 GitHub 账号、姓名和邮箱，不要复制组长的身份配置。

## 四、第一步：接受 GitHub Collaborator 邀请

组长会邀请每位成员加入仓库 `B5CacheVisualizer`。

### 4.1 成员怎样接受邀请

1. 登录自己的 GitHub 账号。
2. 打开 GitHub 发来的邀请邮件，或者查看 GitHub 页面右上角的通知。
3. 打开 `B5CacheVisualizer` 的协作邀请。
4. 确认邀请来自仓库所有者 `wuwingyue2006-droid`。
5. 点击接受邀请的按钮。
6. 接受后打开仓库：<https://github.com/wuwingyue2006-droid/B5CacheVisualizer>。

仓库即使是 Public，任何人也可以查看和 clone；但只有接受 Collaborator 邀请的成员才能向该仓库上传自己的分支。因此，“能看到仓库”或“能 clone”不等于已经获得协作权限。

### 4.2 接受后怎样确认

成员在群里发：

```text
姓名：
GitHub 用户名：
已接受 B5CacheVisualizer Collaborator 邀请：是
```

组长应在仓库的协作者管理页面确认该成员不再处于等待接受邀请的状态。若邀请过期或找不到，让组长重新发送，不要改用别人的 GitHub 账号。

## 五、第二步：安装并检查 Git

打开 PowerShell。可以按 `Win` 键，搜索“PowerShell”，然后打开。

输入：

```powershell
git --version
```

如果出现类似下面的内容，说明 Git 已安装：

```text
git version 2.xx.x.windows.x
```

如果提示无法识别 `git`、找不到命令或不是内部命令：

1. 安装 Git for Windows；
2. 安装时不确定的选项保持默认即可；
3. 安装完成后关闭当前 PowerShell；
4. 重新打开 PowerShell；
5. 再运行 `git --version`。

不要在同一个报错窗口中反复尝试，也不需要运行 `git init`。

## 六、第三步：配置自己的 Git 身份

每位成员只在自己的电脑上配置自己的身份。下面的“你的名字”和“你的邮箱”必须替换，不能原样复制。

```powershell
git config --global user.name "你的名字或常用英文名"
git config --global user.email "你的邮箱"
```

例如：

```powershell
git config --global user.name "zhangsan"
git config --global user.email "zhangsan@example.com"
```

建议邮箱使用该成员 GitHub 账号中已经验证的邮箱。若成员在 GitHub 开启了邮箱隐私保护，可以到 GitHub 的邮箱设置中复制 GitHub 提供的 `noreply` 邮箱。

分别检查配置：

```powershell
git config --global user.name
git config --global user.email
```

屏幕上应该显示本人的姓名和邮箱。如果显示的是其他人的信息，重新运行配置命令进行更正。

这两个配置决定以后提交记录显示的作者，但它们不是 GitHub 登录密码，也不会替代 Collaborator 权限。

## 七、第四步：把仓库 clone 到自己的电脑

### 7.1 选择父目录

以下示例把项目放在 `D:\Projects`。如果你选择其他位置，后续路径也要对应修改。

先在资源管理器中创建 `D:\Projects`，然后在 PowerShell 输入：

```powershell
cd D:\Projects
```

可以用下面的命令确认当前位置：

```powershell
Get-Location
```

### 7.2 clone 仓库

输入：

```powershell
git clone https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git
```

完成后进入项目：

```powershell
cd B5CacheVisualizer
```

检查当前目录内容：

```powershell
Get-ChildItem -Force
```

应该能够看到 `.git`、`.gitignore` 和 `README.md`。

注意：

- `.git` 是隐藏目录，普通资源管理器视图中可能看不到，这是正常的。
- 不要进入项目后再次运行 `git init`。
- 不要把其他同名项目直接复制进来。
- 如果提示目标目录 `B5CacheVisualizer` 已经存在且不为空，先停止操作并确认这个目录的来源，不要强行覆盖或删除。

## 八、第五步：检查仓库是否连接正确

确保 PowerShell 当前位于 `B5CacheVisualizer` 目录，然后运行：

```powershell
git status
```

正常情况下应看到类似：

```text
On branch main
Your branch is up to date with 'origin/main'.
nothing to commit, working tree clean
```

再检查远程仓库：

```powershell
git remote -v
```

应看到 `origin` 的 fetch 和 push 地址都是：

```text
https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git
```

再查看所有本地和远程分支：

```powershell
git branch -a
```

当前至少应看到：

```text
* main
  remotes/origin/HEAD -> origin/main
  remotes/origin/dev
  remotes/origin/main
```

如果远程地址不是上面的地址，停止操作并把 `git remote -v` 的完整结果发给组长，不要自行向陌生仓库 push。

## 九、第六步：切换到团队开发基线 dev

先尝试：

```powershell
git switch dev
```

第一次切换时，Git 通常会自动根据远程的 `origin/dev` 创建本地 `dev` 并建立跟踪关系。

如果提示本地不存在 `dev`，运行：

```powershell
git switch --track origin/dev
```

然后拉取最新内容：

```powershell
git pull --ff-only
```

最后连续检查：

```powershell
git branch --show-current
git status
```

正确结果应满足：

- `git branch --show-current` 输出 `dev`；
- `git status` 显示当前位于 `dev`；
- 工作区是干净的；
- 本地 `dev` 与 `origin/dev` 保持同步。

## 十、现在必须停在这里

全组现阶段统一只做到：

```text
安装 Git
↓
配置本人姓名和邮箱
↓
接受 Collaborator 邀请
↓
clone B5CacheVisualizer
↓
切换到 dev
↓
拉取最新内容
↓
停止，等待分工
```

现在不要执行以下操作：

- 不要创建任何 `feature-*` 分支；
- 不要修改 `README.md` 或其他仓库文件；
- 不要提交空 commit；
- 不要向 `main` 或 `dev` 直接 push；
- 不要尝试用 `--force` 强制上传；
- 不要把自己的测试文件、安装包或构建产物放进仓库。

完成后，每位成员把下面这段填写完整并发到群里：

```text
【B5 GitHub 接入确认】
姓名：
GitHub 用户名：
Git 版本：
本地仓库路径：
当前分支：dev
git status 是否显示工作区干净：是/否
是否已经接受 Collaborator 邀请：是/否
遇到的问题：无/请写明完整报错
```

建议同时附上这三条命令的输出截图：

```powershell
git --version
git branch --show-current
git status
```

截图前检查屏幕内容，不要公开个人访问令牌、密码或其他敏感信息。

## 十一、分工确定以后才执行：创建个人 feature 分支

本节只是以后使用的说明，现在不要执行。

假设组长最终通知某位成员负责 Mapping，并明确分支名为 `feature-mapping`，该成员再执行：

```powershell
git switch dev
git pull --ff-only
git status
git switch -c feature-mapping
git push -u origin feature-mapping
```

每条命令的含义：

1. 回到团队开发基线 `dev`；
2. 获取远程最新的 `dev`；
3. 确认没有未提交修改；
4. 从最新的 `dev` 创建本人分支；
5. 第一次把本人分支上传到 GitHub，并建立跟踪关系。

只有组长明确下发以下四项信息后，成员才能创建分支：

```text
负责人：
模块名称：
正式分支名：
允许创建分支：是
```

分支名必须完全按照通知输入，不能自行改成 `mybranch`、`test`、`final`、中文名称或其他临时名称。

## 十二、分工后的日常开发流程

本节同样在正式分工后才使用。

假设成员的分支是 `feature-mapping`。

### 12.1 每次开始开发前

先确认仓库中没有未提交的临时修改：

```powershell
git status
```

然后更新 `dev`：

```powershell
git switch dev
git pull --ff-only
```

回到自己的分支，并把最新 `dev` 合并进来：

```powershell
git switch feature-mapping
git merge dev
```

可以记成：

```text
开发前：dev → pull → feature → merge dev
```

如果 `git switch dev` 因为未提交修改而失败，不要强行切换。先运行 `git status`，确认这些修改属于哪个功能，再完成、提交或向组长求助。

### 12.2 开发过程中

- 只修改自己任务范围内的文件；
- 每完成一个小功能就编译和测试；
- 不要一次积累几天的大量修改；
- 不要提交 Visual Studio 的个人配置和编译产物；
- 需要改变公共接口、公共头文件或共享数据结构时，先在群里说明影响。

当前仓库的 `.gitignore` 已经忽略常见 Visual Studio 个人文件、`Debug`、`Release`、`x64`、`x86` 和中间构建文件，但提交前仍然必须检查。

### 12.3 完成一个相对完整的小功能后

先查看状态和实际修改：

```powershell
git status
git diff
```

优先只添加本次功能相关文件：

```powershell
git add 文件名
```

如果已经确认当前所有修改都属于同一个功能，也可以：

```powershell
git add .
```

再次检查即将提交的内容：

```powershell
git status
git diff --staged
```

然后提交并上传：

```powershell
git commit -m "Implement direct mapping"
git push
```

可以记成：

```text
开发后：status → diff → add → staged diff → commit → push
```

好的 commit 示例：

```text
Implement direct mapping
Add LRU replacement
Add cache line visualization
Fix invalid cache configuration
Add hit rate statistics
```

不好的 commit 示例：

```text
update
修改
111
final
最终版2
```

一个 commit 尽量只表达一件清楚的事情。不要把“新增功能、界面重做、修复另一个模块”全部塞进同一次提交。

## 十三、阶段功能完成后：通过 Pull Request 合并到 dev

个人模块完成一个阶段后，不直接往 `main` 合并，也不直接向 `dev` push。正确流程是：

```text
feature-mapping
       ↓
Pull Request + 自测说明 + 组员检查
       ↓
dev
```

### 13.1 创建 Pull Request 前

1. 确认代码能够编译；
2. 完成本模块要求的测试；
3. 确认自己的分支已经合并最新 `dev`；
4. 处理发现的问题；
5. 确认所有 commit 已经 push 到远程。

### 13.2 在 GitHub 页面创建 Pull Request

1. 打开仓库页面；
2. 进入 Pull requests；
3. 选择创建新的 Pull Request；
4. `base` 必须选择 `dev`；
5. `compare` 选择自己的 `feature-*` 分支；
6. 检查文件变更，确认没有混入无关文件；
7. 填写清楚标题和说明；
8. 创建 Pull Request；
9. 把 PR 链接发到群里，等待检查。

PR 说明至少包含：

```text
【完成内容】
- 待填写

【主要修改文件】
- 待填写

【测试方法】
- 待填写

【测试结果】
- 待填写

【对公共接口的影响】
- 无 / 请具体说明

【仍存在的问题】
- 无 / 请具体说明
```

创建前必须再次确认：

```text
base: dev ← compare: feature-你的模块
```

如果页面显示 `base: main`，不要创建或合并，先改成 `dev`。

## 十四、全组必须遵守的协作规则

### 规则 1：不要直接在 main 上写代码

`main` 只保留全组确认稳定、可以提交或演示的版本。

### 规则 2：不要直接向 dev 上传未经测试的大量代码

个人代码通过自己的 feature 分支和 Pull Request 进入 `dev`。

### 规则 3：一次提交只做一个相对完整的小功能

提交信息要能让其他成员只看一行就知道本次修改做了什么。

### 规则 4：修改公共接口前先在群里说明

例如准备修改：

```cpp
CacheSimulator::Access(...)
```

不能自己直接改完再通知，因为其他成员的模块可能依赖该接口。讨论时至少说明：

- 为什么要改；
- 原接口是什么；
- 新接口是什么；
- 会影响哪些模块；
- 是否需要其他成员同步修改。

### 规则 5：避免五个人同时大量修改同一个文件

尤其不要让所有人长期同时修改一个大型 `Dlg.cpp`。正式分工时要划分源文件和职责，使每个人主要修改自己的模块。

### 规则 6：出现冲突时不要盲目选择“全部保留”

先运行：

```powershell
git status
```

把冲突文件、完整提示和自己刚才执行的命令发给组长或 AI。没有理解冲突内容前，不要强制 push，不要随意删除别人的实现。

如果只是刚执行 `git merge dev` 后发现冲突，而且还没有手工提交合并结果，可以在确认当前确实处于合并状态后询问组长是否使用：

```powershell
git merge --abort
```

不要把这条命令当作日常撤销按钮。

### 规则 7：不得泄露凭据

- 不要把 GitHub 密码、访问令牌或浏览器授权信息发到群里；
- 不要把访问令牌写进代码、配置文件或 commit；
- 不要让 AI 输出、保存或上传自己的访问令牌；
- 截图报错前检查是否包含敏感信息。

## 十五、第一次 push 时的 GitHub 登录说明

因为仓库是 Public，clone 阶段可能不要求登录；真正第一次 push 自己的 feature 分支时，才可能出现 GitHub 身份验证。

在 Windows 上通常会由 Git Credential Manager 打开浏览器，成员应：

1. 确认浏览器登录的是自己的 GitHub 账号；
2. 按页面提示授权 Git；
3. 完成后返回 PowerShell；
4. 等待 push 结束；
5. 到 GitHub 仓库的 Branches 页面确认自己的分支已经出现。

GitHub 的 HTTPS Git 操作不能使用普通账号密码代替令牌。如果出现用户名和密码提示，不要在群里询问或发送密码，优先取消并检查 Git for Windows / Git Credential Manager 是否正确安装，再把不含敏感信息的完整提示发给 AI 或组长。

即使登录成功，如果 Collaborator 邀请没有接受，push 仍可能显示 403 或没有写权限。

## 十六、常见问题处理

### 16.1 `git` 无法识别

原因通常是未安装 Git，或安装后没有重新打开 PowerShell。安装 Git for Windows并重开终端，再运行 `git --version`。

### 16.2 `Repository not found`

依次确认：

1. 仓库地址是否完整；
2. 是否登录了正确的 GitHub 账号；
3. 网络是否正常；
4. 组长是否更改了仓库可见性或地址。

不要反复输入密码。

### 16.3 clone 时提示目标文件夹已存在

不要覆盖。先运行：

```powershell
Get-Location
Get-ChildItem -Force
```

把结果交给 AI 或组长判断现有目录能否使用。

### 16.4 `git switch dev` 提示找不到分支

先运行：

```powershell
git fetch origin
git branch -a
```

如果能看到 `remotes/origin/dev`，再运行：

```powershell
git switch --track origin/dev
```

如果仍然看不到 `origin/dev`，把完整输出发给组长。

### 16.5 push 显示 403、permission denied 或无写入权限

依次确认：

1. 是否接受了 Collaborator 邀请；
2. 浏览器登录的是否是收到邀请的账号；
3. 远程地址是否是组长的仓库；
4. 组长是否能在协作者页面看到该账号已经加入。

不要用 `--force`，也不要改成向 `main` push 来“试一下”。

### 16.6 切换分支时提示会覆盖本地修改

立即停止。运行：

```powershell
git status
git diff
```

把输出和这些修改的来源说明给 AI 或组长。不要在不理解后果时执行清理、还原或强制切换命令。

### 16.7 不确定自己目前在哪个仓库或分支

运行：

```powershell
Get-Location
git remote -v
git branch --show-current
git status
```

这四条输出通常足以让组长或 AI 判断当前状态。

## 十七、可以直接交给每位成员 AI 的接入提示词

每位成员可以把下面整段复制给自己的 AI。AI 应根据成员贴回的命令输出一步一步继续，不要一次让成员执行全部命令。

```text
你现在是我的 GitHub 协作接入助手。我使用 Windows 和 PowerShell，要加入课程设计仓库：
https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git

本轮唯一目标：
1. 检查 Git；
2. 配置我本人的 Git 姓名和邮箱；
3. 确认我已接受 Collaborator 邀请；
4. 把仓库 clone 到我指定的父目录；
5. 核对 origin 地址；
6. 切换到 dev；
7. 拉取最新 dev；
8. 确认工作区干净，然后停止。

严格限制：
- 不创建任何 feature 分支；
- 不修改仓库文件；
- 不执行 git init；
- 不 commit；
- 不 push；
- 不直接操作 main 或 dev 的远程内容；
- 不使用 --force；
- 不删除或覆盖已有目录；
- 不索要、显示或保存我的密码和访问令牌。

指导方式：
- 每次只给我 1 个可复制执行的 PowerShell 命令；
- 先用一句话解释该命令用途和正常结果；
- 等我贴回完整输出后再给下一步；
- 如果输出异常，先诊断，不要跳到后面的步骤；
- 涉及路径时先用 Get-Location 确认；
- 涉及仓库时先用 git status、git remote -v 和 git branch -a 确认；
- 不确定会不会丢失文件时必须停止并提醒我联系组长。

全部完成后，请让我运行并汇总：
git --version
git config --global user.name
git config --global user.email
Get-Location
git remote -v
git branch --show-current
git status

现在先问我：是否已接受邀请、准备使用的本人姓名和邮箱、计划把项目放在哪个父目录。然后从 git --version 开始，一步一步指导我。
```

## 十八、组长接入验收清单

组长对每位成员逐项确认：

- [ ] GitHub 用户名已经登记；
- [ ] Collaborator 邀请已经接受；
- [ ] `git --version` 正常；
- [ ] Git 作者姓名和邮箱是成员自己的；
- [ ] clone 的远程地址正确；
- [ ] 成员当前分支是 `dev`；
- [ ] 成员的工作区干净；
- [ ] 成员没有提前创建 feature 分支；
- [ ] 成员知道不能直接在 `main` 和 `dev` 上开发；
- [ ] 成员知道出现报错时要提供完整命令和完整输出；
- [ ] 五个人全部通过后，再开始模块、接口和验收标准分工。

全员通过接入验收以后，下一步一次性确定三层内容：

1. 五个人分别负责什么；
2. 每个模块的输入、输出和公共接口是什么；
3. 每个人具体做到什么程度才算完成。

之后再为每位成员生成一份可直接交给本人及其 AI 的详细任务包，避免后期出现五套互不兼容的实现。
