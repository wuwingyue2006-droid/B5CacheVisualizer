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
}

}  // namespace

void AddStatisticsTests(TestList& tests) {
    tests.push_back({"Statistics: counters and rates", TestStatisticsCountersAndRates});
    // TODO(E): add empty-trace, reset and report-format cases here.
}

}  // namespace b5cache::tests
