# 成员 B 任务卡：Mapping

## 身份信息

```text
成员代号：B
负责模块：Direct / Fully Associative / Set Associative 地址映射
个人分支：feature-mapping
前置条件：已确认代号 B；从最新 dev 创建 feature-mapping；工作区状态已检查
```

## 允许主要修改的文件

```text
src/mapping/IMappingStrategy.h
src/mapping/MappingStrategies.cpp
B5CacheCoreTests/MappingTests.cpp
```

一般不需要修改 `IMappingStrategy.h`。现有接口已经足够时，只修改实现和测试。

## 禁止自行修改

```text
src/core/
src/replacement/
src/statistics/
src/trace/
B5CacheVisualizer/
src/common/CacheTypes.h
其他成员测试文件
```

## 输入与输出

输入：

```cpp
blockNumber
lineCount
associativity
```

输出：

```cpp
AddressMapping {
    setIndex,
    tag,
    setCount
}
```

Mapping 不读取 Cache Line，不选择 victim，不维护命中率。

## 三种映射的统一公式

### Direct Mapping

```text
associativity = 1
setCount = lineCount
setIndex = blockNumber % setCount
tag = blockNumber / setCount
```

骨架已经实现，成员 B 负责复核边界测试。

### Fully Associative

```text
associativity = lineCount
setCount = 1
setIndex = 0
tag = blockNumber
```

### Set Associative

```text
1 < associativity < lineCount
setCount = lineCount / associativity
setIndex = blockNumber % setCount
tag = blockNumber / setCount
```

## 具体任务

1. 实现 `FullyAssociativeMappingStrategy::Locate()`；
2. 实现 `SetAssociativeMappingStrategy::Locate()`；
3. 保留并复核 Direct Mapping；
4. 对 lineCount、associativity 和整除关系做防御性校验；
5. 非法输入抛出 `std::invalid_argument`；
6. 未知 MappingKind 继续由工厂拒绝；
7. 移除完成部分的 `TODO(B)` 和 `logic_error`；
8. 不改变工厂函数和接口签名。

## 必须测试的例子

在 `MappingTests.cpp` 中至少加入：

### Direct

```text
block=9, lines=8, ways=1
期望 sets=8, set=1, tag=1
```

### Fully Associative

```text
block=9, lines=8, ways=8
期望 sets=1, set=0, tag=9
```

### 2-way Set Associative

```text
block=9, lines=8, ways=2
期望 sets=4, set=1, tag=2
```

### 4-way Set Associative

```text
block=15, lines=16, ways=4
期望 sets=4, set=3, tag=3
```

还要测试：

- lineCount 为 0；
- associativity 为 0；
- associativity 不能整除 lineCount；
- Direct 的 ways 不为 1；
- Fully 的 ways 不等于 lines；
- Set 的 ways 为 1 或等于 lines。

## 完成标准

- [ ] 三种 Mapping 均可通过工厂创建；
- [ ] 三种 Locate 均返回正确 set、tag、setCount；
- [ ] 无效输入明确报错；
- [ ] `TODO(B)` 已移除；
- [ ] `MappingTests.cpp` 覆盖正常和异常输入；
- [ ] 完整测试全部通过；
- [ ] 没有修改 Core 或 UI 来“适配”错误公式。

## 建议提交信息

```text
Implement associative mapping strategies
```

## 交给成员 B 的 AI 提示词

```text
你正在 B5CacheVisualizer 仓库中协助成员 B 完成 Mapping 模块。

完整读取 AGENTS.md、docs/ARCHITECTURE.md、docs/INTERFACES.md、docs/OWNERSHIP.md、docs/tasks/B-mapping.md。先检查 Get-Location、git status、git remote -v、git branch --show-current，当前分支必须是 feature-mapping。

只修改 src/mapping/ 和 B5CacheCoreTests/MappingTests.cpp。按照任务卡给定公式实现 Fully Associative 和 Set Associative，不自行修改 Core、UI、公共类型或其他测试。每次只完成一个小步骤并运行测试。最终运行 scripts/test.ps1，报告结果和建议 commit 信息，不合并 dev/main。
```
