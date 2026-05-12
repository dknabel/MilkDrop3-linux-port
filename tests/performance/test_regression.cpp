#include <gtest/gtest.h>
#include "core/profiler.h"
#include "core/performance_baseline.h"
#include <iostream>
#include <filesystem>

TEST(RegressionTest, NoPerformanceRegression) {
  // Load baseline metrics from JSON
  // Try multiple possible locations
  std::string baselinePath;
  const std::vector<std::string> possiblePaths = {
    "baseline.json",
    "../baseline.json",
    "tests/performance/baseline.json",
    "../../tests/performance/baseline.json"
  };

  for (const auto& path : possiblePaths) {
    if (std::filesystem::exists(path)) {
      baselinePath = path;
      break;
    }
  }

  ASSERT_FALSE(baselinePath.empty()) << "Could not find baseline.json in expected locations";
  ASSERT_TRUE(milkdrop::PerformanceBaseline::load(baselinePath));

  auto& prof = milkdrop::Profiler::instance();

  // Get collected metrics
  std::vector<std::string> metricNames = {
    "eval_simple",
    "eval_complex",
    "render_commands",
    "e2e_frame",
    "e2e_multi_preset",
    "e2e_sustained"
  };

  std::cout << "\n=== Performance Regression Report ===\n";

  int regressionCount = 0;
  for (const auto& name : metricNames) {
    auto metrics = prof.getMetrics(name);
    if (metrics) {
      double avgMs = metrics->avgMs();
      milkdrop::PerformanceBaseline::printComparisonReport(name, avgMs);

      if (milkdrop::PerformanceBaseline::exceedsBaseline(name, avgMs)) {
        regressionCount++;
      }
    } else {
      std::cout << "  WARNING: No metrics for " << name << "\n";
    }
  }

  std::cout << "\nRegressions detected: " << regressionCount << "\n";

  EXPECT_EQ(regressionCount, 0) << "Performance regressions detected!";
}
