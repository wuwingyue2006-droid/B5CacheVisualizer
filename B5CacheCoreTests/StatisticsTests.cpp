#include "TestSuites.h"

#include "statistics/CacheStatistics.h"

#include <cmath>

namespace b5cache::tests {
namespace {

void TestStatisticsCountersAndRates() {
    CacheStatistics statistics;

    AccessResult l1Hit;
    l1Hit.outcome = AccessOutcome::L1Hit;
    statistics.Record(l1Hit);

    AccessResult l2Hit;
    l2Hit.request.isWrite = true;
    l2Hit.outcome = AccessOutcome::L2Hit;
    statistics.Record(l2Hit);

    AccessResult miss;
    miss.outcome = AccessOutcome::MemoryMiss;
    statistics.Record(miss);

    const auto snapshot = statistics.Snapshot();
    Require(snapshot.accesses == 3, "Statistics should count all accesses.");
    Require(snapshot.reads == 2 && snapshot.writes == 1, "Statistics should split reads and writes.");
    Require(snapshot.l1Hits == 1 && snapshot.l2Hits == 1 && snapshot.memoryMisses == 1,
            "Statistics should count every outcome.");
    Require(std::abs(snapshot.OverallHitRate() - (2.0 / 3.0)) < 0.000001,
            "Overall hit rate is incorrect.");
    Require(std::abs(snapshot.MissRate() - (1.0 / 3.0)) < 0.000001,
            "Miss rate is incorrect.");
    Require(snapshot.accesses == snapshot.reads + snapshot.writes,
            "Access count must equal reads plus writes.");
    Require(snapshot.accesses == snapshot.l1Hits + snapshot.l2Hits + snapshot.memoryMisses,
            "Access count must equal the sum of outcomes.");
}

void TestEmptyStatisticsAndReset() {
    CacheStatistics statistics;
    const auto empty = statistics.Snapshot();
    Require(empty.accesses == 0 && empty.reads == 0 && empty.writes == 0,
            "New statistics must start at zero.");
    Require(empty.L1HitRate() == 0.0 && empty.L2HitRate() == 0.0 &&
                empty.OverallHitRate() == 0.0 && empty.MissRate() == 0.0,
            "Empty statistics must report zero rates.");

    AccessResult result;
    result.request.isWrite = true;
    result.outcome = AccessOutcome::L1Hit;
    statistics.Record(result);
    statistics.Reset();

    const auto reset = statistics.Snapshot();
    Require(reset.accesses == 0 && reset.reads == 0 && reset.writes == 0 &&
                reset.l1Hits == 0 && reset.l2Hits == 0 && reset.memoryMisses == 0,
            "Reset must clear every counter.");
}

}  // namespace

void AddStatisticsTests(TestList& tests) {
    tests.push_back({"Statistics: counters and rates", TestStatisticsCountersAndRates});
    tests.push_back({"Statistics: empty state and reset", TestEmptyStatisticsAndReset});
}

}  // namespace b5cache::tests
