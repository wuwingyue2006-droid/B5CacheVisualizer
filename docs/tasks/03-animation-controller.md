# 阶段 03 任务卡：动画控制器

## 阶段信息

```text
顺序：03
分支：feature-03-animation-controller
主题：访问历史、前后步进、自动播放、暂停与速度控制
前置：阶段 02 已合入 dev；开始编码前先把最新 origin/dev 合入本分支
状态：待开发
```

本阶段只负责让已经正确的 Cache 访问结果能够按时间线播放和回看，不修改映射、替换、统计或两级访问算法。

## 开发目标

- 引入轻量 `VisualizationFrame`，保存一次访问所需的只读展示快照：请求、`AccessResult`、统计、L1/L2 行状态和帧序号。
- 引入展示控制器，统一管理时间线、当前帧、播放状态和播放速度。
- 增加“上一步”“下一步/单步”“自动播放”“暂停”“停止”操作。
- 增加速度选择，至少提供慢速、正常、快速三档。
- 使用一个 MFC Timer 推进播放；任何时候最多只能存在一个播放 Timer。
- 回看历史帧只改变界面展示，不重新改写 Cache 核心状态。
- 当前帧改变时，同步刷新地址拆分、访问路径、L1/L2 表格、统计文字和图表。

## 建议文件边界

允许主要修改：

```text
B5CacheVisualizer/B5CacheVisualizerDlg.h
B5CacheVisualizer/B5CacheVisualizerDlg.cpp
B5CacheVisualizer/B5CacheVisualizer.rc
B5CacheVisualizer/resource.h
docs/tasks/03-animation-controller.md
```

建议新增独立展示层文件，避免继续扩大对话框类：

```text
B5CacheVisualizer/VisualizationFrame.h
B5CacheVisualizer/VisualizationController.h
B5CacheVisualizer/VisualizationController.cpp
```

新增 `.cpp` 后需要同步更新 MFC 项目文件；这是工程文件变更，提交前应在变更说明中单独列出。不得修改 `src/common/` 公共结构，也不得在展示控制器中复制 Cache 算法。

## 实现顺序

1. 定义 `VisualizationFrame`，复制保存 L1/L2 只读状态，禁止持有会失效的核心引用。
2. 实现时间线的清空、追加、定位上一帧/下一帧以及首尾边界判断。
3. 把现有 Step 接入控制器：已有帧用于回看，没有下一帧时才执行新的核心访问。
4. 增加单一 Timer 的开始、暂停、继续和停止逻辑。
5. 把速度档位映射为 Timer 间隔，切换速度时安全重建当前 Timer。
6. 统一按钮启用状态：播放时禁止 Apply、导入、清空 Trace 和重复启动播放。
7. Apply Config、Reset、Trace 文本改变或重新导入时，停止 Timer 并使旧时间线失效。
8. 到达 Trace 末尾后自动停止，保持最后一帧可见，不越界。

## 状态约束

```text
Stopped：可修改配置和 Trace，可从头播放
Playing：Timer 正在推进，禁止产生第二个 Timer
Paused：保留当前位置，可继续或停止
Reviewing：查看历史帧，不修改模拟器当前真实状态
```

无论从哪一种状态执行 Reset 或 Apply Config，都必须先终止 Timer、清空历史帧和旧高亮，再建立新会话。

## 不属于本阶段

- 不开发 Trace 生成器；
- 不开发策略对比；
- 不开发文件导出；
- 不修改核心命中、映射、替换或统计规则；
- 不处理最终 Release、跨电脑、打包和集中验收。

## 开发完成判定

- 可以稳定地前进和回看已生成的帧；
- 自动播放、暂停、继续、停止和速度切换都有唯一且明确的状态；
- 回看帧与当时的地址、路径、Cache 表和统计一致；
- Reset、Apply Config、Trace 变更后不会继续播放旧时间线；
- 对话框只负责事件连接与刷新，时间线状态集中在展示控制器中；
- 没有修改核心公共接口。

## 建议提交信息

```text
Add cache access playback controller
```
