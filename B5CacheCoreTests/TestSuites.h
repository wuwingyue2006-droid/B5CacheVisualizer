#pragma once

#include "TestSupport.h"

namespace b5cache::tests {

void AddCoreTests(TestList& tests);
void AddMappingTests(TestList& tests);
void AddReplacementTests(TestList& tests);
void AddStatisticsTests(TestList& tests);
void AddTraceTests(TestList& tests);
void AddComparisonTests(TestList& tests);

}  // namespace b5cache::tests
