# B5CacheVisualizer 程序交付说明

## 提交内容

最终 ZIP 应同时包含：

- `Release/B5CacheVisualizer.exe`；
- Visual Studio 解决方案和完整源码；
- README；
- 示例 Trace；
- 自动化测试源码；
- 架构、接口和验收文档。

## 运行方法

1. 解压 ZIP；
2. 打开 `Release` 文件夹；
3. 双击 `B5CacheVisualizer.exe`；
4. 使用默认 Trace 或从 `examples` 导入示例；
5. 如果 Windows 显示来源提醒，先核对文件来自本小组提交包，再由使用者决定是否运行。

程序采用静态 MFC 和静态 Release C++ 运行库，不要求目标电脑安装 Visual Studio。操作系统仍需提供标准 Windows 系统组件。

## 建议演示顺序

1. 导入 `examples/demo-complete.trace`，逐步显示三种访问结果和 Cache Line 更新；
2. 切换 Auto、Pause 和播放速度；
3. 打开 Trace Generator，展示局部性模式和固定随机种子；
4. 打开 Compare，载入教学预设和冲突 Trace；
5. 运行对比并观察图表；
6. 导出 CSV 或 TXT。

## 提交前检查

- ZIP 小于 25 MB；
- ZIP 中存在 Release exe；
- ZIP 中不存在 `.git`、`.vs`、`obj`、PDB、日志和用户配置；
- 本机 Release 测试显示 90/90；
- 至少两名组员完成跨电脑复验；
- 中文路径、相对路径、1366 x 768 和高 DPI 结果已记录。

## 已知模型边界

程序用于 Cache 原理教学和配置对比，不模拟流水线、TLB、硬件预取、多核一致性及精确延迟。后续实验分析应把模拟结果表述为本模型内的相对比较。
