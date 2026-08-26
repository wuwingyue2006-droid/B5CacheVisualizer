# B5 组员 Git 一步一步操作卡

> 这份操作卡只要求你会复制命令、看有没有报错。
> 不要求你理解复杂 Git 原理。遇到任何异常，立即停止，把完整画面发给 AI 或组长。

## 一、全组统一规则

你只需要记住下面四件事：

1. 不在 `main` 上写代码。
2. 不在 `dev` 上写代码。
3. 只在组长分配给你的 `feature-*` 分支上写代码。
4. 任何命令出现红色报错，立即停止，不要自己乱试其他命令。

以下危险命令，组员不要自行执行：

```text
git push --force
git reset --hard
git clean -fd
git rebase
git cherry-pick
```

也不要删除 `.git` 文件夹，不要为了修复问题反复重新 clone。

## 二、第一次接入仓库

第一次接入时，按照组长发送的《B5 组员第一次加入 GitHub 项目流程》或让 AI 一步一步带你完成。

第一次完成：

```text
接受 GitHub 邀请
    ↓
安装 Git
    ↓
配置自己的姓名和邮箱
    ↓
clone 仓库
    ↓
切换到 dev
    ↓
从最新 dev 创建本人固定 feature 分支
    ↓
第一次 push 后向组长报告
```

五个分支名已经固定。成员确认自己的 A–E 代号后，可以随时从最新 `dev` 创建对应分支，不需要等待组长单独批准。

## 三、组长必须单独发给你的信息

开始前确认下面这张任务卡信息：

```text
【B5 个人 Git 任务卡】
姓名：
负责模块：
本地仓库路径：
个人分支名：feature-________
主要负责文件或文件夹：
本阶段任务：
本阶段完成标准：
```

如果“个人分支名”或“主要负责文件”没有填写完整，不要自己猜，先问组长。

## 四、完成接入后创建自己的分支

下面使用 `feature-mapping` 举例。你必须把它替换成组长分配给你的真实分支名。

### 第 1 步：进入项目目录

示例：

```powershell
cd D:\Projects\B5CacheVisualizer
```

你的仓库不在这里时，必须使用你自己的真实路径。

### 第 2 步：确认进入了正确仓库

```powershell
git status
```

没有报错才能继续。如果提示“不是 Git 仓库”，立即停止。

### 第 3 步：回到 dev

```powershell
git switch dev
```

### 第 4 步：下载最新 dev

```powershell
git pull --ff-only
```

### 第 5 步：确认工作区干净

```powershell
git status
```

正常结果应包含：

```text
nothing to commit, working tree clean
```

### 第 6 步：创建组长分配的个人分支

示例：

```powershell
git switch -c feature-mapping
```

### 第 7 步：第一次上传个人分支

示例：

```powershell
git push -u origin feature-mapping
```

第一次 push 可能打开浏览器，登录并授权自己的 GitHub 账号即可。不要把账号密码或访问令牌发给任何人。

### 第 8 步：确认当前分支

```powershell
git branch --show-current
```

屏幕上必须显示组长分配给你的分支名，例如：

```text
feature-mapping
```

把这一结果截图发给组长。至此，第一次创建分支完成。

## 五、每次开始写代码前

每次开始开发时，把下面流程交给 AI，让 AI 一条一条带你执行。

### 第 1 步：进入项目

```powershell
cd 你的本地仓库路径
```

### 第 2 步：检查有没有上次遗留的修改

```powershell
git status
```

如果显示工作区不干净，不要继续切换分支。把完整输出发给 AI。

如果显示工作区干净，继续：

```powershell
git switch dev
```

### 第 3 步：获取团队最新代码

```powershell
git pull --ff-only
```

### 第 4 步：回到自己的分支

以下仍以 `feature-mapping` 为例：

```powershell
git switch feature-mapping
```

### 第 5 步：把最新 dev 合并到自己的分支

```powershell
git merge dev
```

没有报错才能开始写代码。

如果出现 `CONFLICT`、`conflict`、`Automatic merge failed` 或红色报错：

1. 不要关闭窗口；
2. 不要随便修改冲突符号；
3. 不要 commit；
4. 不要 push；
5. 运行 `git status`；
6. 把完整输出发给组长和 AI。

## 六、写代码时只做自己的任务

开发时遵守：

- 只完成任务卡上的本阶段任务；
- 主要修改分配给自己的文件；
- 不随便移动或重命名别人的文件；
- 不随便修改公共接口；
- 修改公共头文件前先在群里说明；
- 每完成一个小功能就编译和测试；
- 不把 `.vs`、`Debug`、`Release`、`x64` 等生成内容提交进仓库。

如果 AI 建议你大规模重构、重命名很多文件或修改公共接口，先把方案发给组长确认，不要立即执行。

## 七、完成一个小功能后上传

先保证程序能够正常编译，并完成任务要求的测试。

### 第 1 步：确认自己仍在个人分支

```powershell
git branch --show-current
```

必须显示你的 `feature-*` 分支。

如果显示 `main` 或 `dev`，立即停止，不要继续提交。

### 第 2 步：查看修改

```powershell
git status
```

把输出交给 AI，让 AI 检查是否混入了不应该提交的文件。

### 第 3 步：添加本次修改

只有确认所有修改都属于本次任务后才执行：

```powershell
git add .
```

### 第 4 步：再次检查

```powershell
git status
```

如果出现 `.vs`、`Debug`、`Release`、`x64`、大型安装包或不认识的文件，先停止并询问 AI。

### 第 5 步：提交

提交信息由 AI 根据本次实际功能生成。示例：

```powershell
git commit -m "Implement direct mapping"
```

不要使用下面这类无意义名称：

```text
update
修改
111
final
最终版2
```

### 第 6 步：上传到自己的远程分支

```powershell
git push
```

没有报错后，把下面信息发给组长：

```text
【B5 阶段功能上传】
姓名：
个人分支：
本次完成功能：
测试方法：
测试结果：
commit 信息：
git push 是否成功：是
需要组长检查的问题：无/请写明
```

上传到自己的 feature 分支不等于已经合并进 `dev`。是否合并由组长统一检查和处理。

## 八、组员不负责复杂合并

组员完成代码后只需要：

```text
确认个人分支
    ↓
检查修改
    ↓
commit
    ↓
push 到个人分支
    ↓
通知组长
```

之后由组长负责：

- 检查成员提交；
- 检查公共接口；
- 检查是否影响其他模块；
- 创建或审核 Pull Request；
- 处理合并冲突；
- 把 feature 分支合并到 `dev`；
- 完成集成测试。

组员不要自行把个人分支合并到 `dev` 或 `main`。

## 九、每天只记住两套固定动作

### 开发前

```text
status
→ 切换 dev
→ pull
→ 切换个人 feature
→ merge dev
→ 开始写代码
```

### 开发后

```text
确认个人 feature
→ status
→ add
→ status
→ commit
→ push
→ 通知组长
```

任何一步报错：

```text
立即停止
→ 不执行其他修复命令
→ 复制完整命令和完整输出
→ 发给 AI 或组长
```

## 十、每次都可以复制给 AI 的提示词

```text
你是我的 B5 项目 Git 操作助手。我不熟悉 Git，请严格一条命令一步地指导我。

仓库：
https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git

我的本地仓库路径：
【在这里填写】

组长分配给我的个人分支：
feature-【在这里填写】

本轮目标：
【填写“开始今天的开发”或“提交并上传今天完成的功能”】

必须遵守：
1. 每次只给我一个可以复制的 PowerShell 命令。
2. 等我贴回完整输出后，再判断下一步。
3. 每次 commit 前必须确认 git branch --show-current 是我的 feature 分支。
4. 每次 git add 前先检查 git status。
5. 不允许直接向 main 或 dev 提交和 push。
6. 不允许使用 git push --force、git reset --hard、git clean -fd、rebase 或 cherry-pick。
7. 不删除文件，不覆盖目录，不重新 clone，除非组长明确同意。
8. 出现冲突或任何报错时立即停止，先解释原因。
9. 不索要、输出或保存我的 GitHub 密码和访问令牌。
10. 如果发现当前状态不符合组长规范，明确告诉我停止并联系组长。

现在先让我运行 Get-Location、git status、git remote -v 和 git branch --show-current。根据输出确认环境正确后，再开始本轮操作。
```

## 十一、组长最终验收标准

对普通组员，不要求能够独立处理复杂 Git 问题。只要求每个人做到：

- [ ] 能进入自己的本地仓库；
- [ ] 能确认当前分支；
- [ ] 能在 AI 指导下更新 `dev`；
- [ ] 能切换回自己的 feature 分支；
- [ ] 能在提交前检查 `git status`；
- [ ] 能写出有意义的 commit；
- [ ] 能把代码 push 到自己的分支；
- [ ] 知道不能直接修改 `main` 和 `dev`；
- [ ] 遇到报错会立即停止并保留完整输出；
- [ ] 不会自行执行危险的强制命令。

达到这些要求，就已经足够参与本项目的规范协作。复杂 Git 操作统一交给组长和 AI 处理。
