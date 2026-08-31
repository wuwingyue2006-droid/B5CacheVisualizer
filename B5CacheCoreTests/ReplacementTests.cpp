#include "TestSuites.h"
#include "replacement/IReplacementPolicy.h"

#include <stdexcept>
#include <vector>

namespace b5cache::tests {

namespace {

// ---------------------------------------------------------------------------
// CacheLineState 字段初始化顺序（与 CacheTypes.h 保持一致）：
//   { valid, dirty, tag, blockNumber, insertedAt, lastUsedAt }
// ---------------------------------------------------------------------------

// ===================== FIFO 测试 =====================

void TestFifoVictim() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    const std::vector<CacheLineState> lines = {
        {true, false, 0, 0, 8, 20},   // insertedAt=8
        {true, false, 1, 1, 3, 30},   // insertedAt=3  ← 最早进入，应被淘汰
        {true, false, 2, 2, 5, 10},   // insertedAt=5
    };
    Require(policy->SelectVictim(lines) == 1,
            "FIFO should select the earliest inserted line (smallest insertedAt).");
}

void TestFifoEmptyThrows() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    bool threw = false;
    try {
        (void)policy->SelectVictim({});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw, "FIFO SelectVictim on empty set must throw std::invalid_argument.");
}

void TestFifoSingleLine() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    const std::vector<CacheLineState> lines = {
        {true, false, 0, 0, 42, 99},
    };
    Require(policy->SelectVictim(lines) == 0,
            "FIFO on a single-line set must return index 0.");
}

void TestFifoTieSmallerIndex() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    // 三行 insertedAt 都相同（= 5），平局必须取最小编号
    const std::vector<CacheLineState> lines = {
        {true, false, 10, 10, 5, 100},
        {true, false, 20, 20, 5, 50},
        {true, false, 30, 30, 5, 1},
    };
    Require(policy->SelectVictim(lines) == 0,
            "FIFO with equal insertedAt must select the smaller index (stable).");
}

void TestFifoHitDoesNotChangeVictim() {
    // 核心规则：FIFO 不因 Hit 改变淘汰顺序（insertedAt 不变）
    auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    std::vector<CacheLineState> lines = {
        {true, false, 0, 0, 1, 10},   // 最早进入 (insertedAt=1)
        {true, false, 1, 1, 2, 11},   // insertedAt=2
        {true, false, 2, 2, 3, 12},   // insertedAt=3
    };

    // 初始 victim 应该是 index 0
    Require(policy->SelectVictim(lines) == 0, "Before hit, FIFO victim = index 0.");

    // 命中最早进入的 line（index 0），tick=100
    // FIFO 规则：insertedAt 保持不变，victim 仍是 index 0
    policy->OnHit(lines[0], 100);

    Require(lines[0].insertedAt == 1, "FIFO OnHit must NOT modify insertedAt.");
    Require(lines[0].lastUsedAt == 100, "FIFO OnHit updates lastUsedAt to tick.");
    Require(policy->SelectVictim(lines) == 0,
            "After hitting the oldest line, FIFO victim must remain the same "
            "(insertedAt unchanged — core FIFO invariant).");
}

void TestFifoOnInsertSetsBothFields() {
    auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    CacheLineState line = {};
    policy->OnInsert(line, 77);
    Require(line.insertedAt == 77, "FIFO OnInsert sets insertedAt = tick.");
    Require(line.lastUsedAt == 77, "FIFO OnInsert sets lastUsedAt = tick.");
}

void TestFifoKindReturnsFifo() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Fifo);
    Require(policy->Kind() == ReplacementKind::Fifo,
            "FIFO policy Kind() must return ReplacementKind::Fifo.");
}

// ===================== LRU 测试 =====================

void TestLruVictimByLastUsedAt() {
    // 关键：LRU 使用 lastUsedAt，**不**使用 insertedAt
    const auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    const std::vector<CacheLineState> lines = {
        // insertedAt 故意弄乱，证明 LRU 只看 lastUsedAt
        {true, false, 0, 0, 99, 20},   // lastUsedAt=20  ← 最久未使用
        {true, false, 1, 1, 1,  50},   // lastUsedAt=50
        {true, false, 2, 2, 50, 30},   // lastUsedAt=30
    };
    Require(policy->SelectVictim(lines) == 0,
            "LRU should select the smallest lastUsedAt (index 0), ignoring insertedAt.");
}

void TestLruEmptyThrows() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    bool threw = false;
    try {
        (void)policy->SelectVictim({});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw, "LRU SelectVictim on empty set must throw std::invalid_argument.");
}

void TestLruSingleLine() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    const std::vector<CacheLineState> lines = {
        {true, false, 0, 0, 1, 1},
    };
    Require(policy->SelectVictim(lines) == 0,
            "LRU on a single-line set must return index 0.");
}

void TestLruTieSmallerIndex() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    // 三行 lastUsedAt 都相同（= 25），平局必须取最小编号
    const std::vector<CacheLineState> lines = {
        {true, false, 10, 10, 1, 25},
        {true, false, 20, 20, 2, 25},
        {true, false, 30, 30, 3, 25},
    };
    Require(policy->SelectVictim(lines) == 0,
            "LRU with equal lastUsedAt must select the smaller index (stable).");
}

void TestLruHitUpdatesLastUsedAtAndVictim() {
    // 核心规则：LRU Hit 更新 lastUsedAt，会改变后续 victim
    auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    std::vector<CacheLineState> lines = {
        {true, false, 0, 0, 1, 10},   // lastUsedAt=10  ← 初始 victim
        {true, false, 1, 1, 2, 20},   // lastUsedAt=20
        {true, false, 2, 2, 3, 30},   // lastUsedAt=30
    };

    Require(policy->SelectVictim(lines) == 0,
            "Before hit, LRU victim = index 0 (oldest lastUsedAt).");

    // 命中最久未使用的 line[0]，tick=100 → line[0] 变成最新！
    policy->OnHit(lines[0], 100);

    Require(lines[0].lastUsedAt == 100, "LRU OnHit must update lastUsedAt to tick.");

    // 现在 lastUsedAt 分别是：100, 20, 30 → victim 变成 index 1
    Require(policy->SelectVictim(lines) == 1,
            "After hitting the previously LRU line, LRU victim must shift to the "
            "next-oldest line (index 1, lastUsedAt=20).");
}

void TestLruOnInsertSetsBothFields() {
    auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    CacheLineState line = {};
    policy->OnInsert(line, 42);
    Require(line.insertedAt == 42, "LRU OnInsert sets insertedAt = tick.");
    Require(line.lastUsedAt == 42, "LRU OnInsert sets lastUsedAt = tick.");
}

void TestLruKindReturnsLru() {
    const auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    Require(policy->Kind() == ReplacementKind::Lru,
            "LRU policy Kind() must return ReplacementKind::Lru.");
}

void TestLruMultipleHitSequence() {
    // 模拟一个完整的访问序列，逐步验证 LRU victim 变化
    auto policy = CreateReplacementPolicy(ReplacementKind::Lru);
    std::vector<CacheLineState> lines = {
        // 初始化：tick 1~4 依次插入
        {true, false, 0, 0, 1, 1},   // [0] tick=1
        {true, false, 1, 1, 2, 2},   // [1] tick=2
        {true, false, 2, 2, 3, 3},   // [2] tick=3
        {true, false, 3, 3, 4, 4},   // [3] tick=4  ← 最新插入
    };

    // 初始 victim 应该是 index 0（lastUsedAt=1）
    Require(policy->SelectVictim(lines) == 0,
            "Seq1: initial LRU victim = index 0.");

    // tick=10: 访问 [2]  →  [2] 变最新
    policy->OnHit(lines[2], 10);
    Require(policy->SelectVictim(lines) == 0,
            "Seq2: after hit[2], victim still index 0.");

    // tick=11: 访问 [0]  →  [0] 不再是 LRU！LRU 变成 index 1
    policy->OnHit(lines[0], 11);
    Require(policy->SelectVictim(lines) == 1,
            "Seq3: after hit[0], LRU shifts to index 1.");

    // tick=12: 访问 [1]  →  现在 LRU 应该是 index 3（lastUsedAt=4 最小）
    policy->OnHit(lines[1], 12);
    Require(policy->SelectVictim(lines) == 3,
            "Seq4: after hit[1], LRU = index 3 (only untouched line).");
}

void TestLruVsFifoDifferentVictims() {
    // 相同初始状态，FIFO 和 LRU 选择不同 victim
    // insertedAt: A(1) < B(2) < C(3) → FIFO 淘汰 A
    // lastUsedAt: C(100) < A(200) < B(300) → LRU 淘汰 C
    const std::vector<CacheLineState> lines = {
        {true, false, 0, 0, 1, 200},   // [0] A: 最早进入，但最近用过
        {true, false, 1, 1, 2, 300},   // [1] B: 中间进入，最新
        {true, false, 2, 2, 3, 100},   // [2] C: 最后进入，但最久未用
    };

    const auto fifo = CreateReplacementPolicy(ReplacementKind::Fifo);
    const auto lru  = CreateReplacementPolicy(ReplacementKind::Lru);

    Require(fifo->SelectVictim(lines) == 0,
            "FIFO victim = A (index 0, earliest insertedAt).");
    Require(lru->SelectVictim(lines) == 2,
            "LRU victim = C (index 2, oldest lastUsedAt).");
    Require(fifo->SelectVictim(lines) != lru->SelectVictim(lines),
            "FIFO and LRU should select different victims in this scenario.");
}

// ===================== Factory 测试 =====================

void TestCreateUnknownPolicyThrows() {
    bool threw = false;
    try {
        // 用 enum class 的越界值构造一个未知策略
        (void)CreateReplacementPolicy(static_cast<ReplacementKind>(99));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw,
            "CreateReplacementPolicy with unknown kind must throw std::invalid_argument.");
}

}  // namespace

// ---------------------------------------------------------------------------
// 测试注册（替换原有 TODO(C) 注释，新增全部 LRU 和边界 case）
// ---------------------------------------------------------------------------
void AddReplacementTests(TestList& tests) {
    // —— FIFO（原基础测试 + 新增边界） ——
    tests.push_back({"Replacement: FIFO victim",                        TestFifoVictim});
    tests.push_back({"Replacement: FIFO empty set throws",             TestFifoEmptyThrows});
    tests.push_back({"Replacement: FIFO single-line set",              TestFifoSingleLine});
    tests.push_back({"Replacement: FIFO tie picks smaller index",      TestFifoTieSmallerIndex});
    tests.push_back({"Replacement: FIFO hit keeps victim (invariant)", TestFifoHitDoesNotChangeVictim});
    tests.push_back({"Replacement: FIFO OnInsert fields",              TestFifoOnInsertSetsBothFields});
    tests.push_back({"Replacement: FIFO Kind() returns Fifo",          TestFifoKindReturnsFifo});

    // —— LRU（C 成员实现的核心） ——
    tests.push_back({"Replacement: LRU victim by lastUsedAt (not insertedAt)", TestLruVictimByLastUsedAt});
    tests.push_back({"Replacement: LRU empty set throws",                      TestLruEmptyThrows});
    tests.push_back({"Replacement: LRU single-line set",                       TestLruSingleLine});
    tests.push_back({"Replacement: LRU tie picks smaller index",               TestLruTieSmallerIndex});
    tests.push_back({"Replacement: LRU OnHit changes victim",                  TestLruHitUpdatesLastUsedAtAndVictim});
    tests.push_back({"Replacement: LRU OnInsert fields",                       TestLruOnInsertSetsBothFields});
    tests.push_back({"Replacement: LRU Kind() returns Lru",                    TestLruKindReturnsLru});
    tests.push_back({"Replacement: LRU multi-hit sequence",                    TestLruMultipleHitSequence});
    tests.push_back({"Replacement: LRU vs FIFO choose different victims",      TestLruVsFifoDifferentVictims});

    // —— 工厂函数 ——
    tests.push_back({"Replacement: unknown policy throws", TestCreateUnknownPolicyThrows});
}

}  // namespace b5cache::tests
