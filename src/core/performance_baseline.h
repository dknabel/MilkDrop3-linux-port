#pragma once
#include "performance.h"
#include <unordered_map>
#include <string>

namespace milkdrop {

struct BaselineMetric {
  double avgMs = 0.0;
  double maxMs = 0.0;
  double tolerancePercent = 0.0;

  // Check if current metric exceeds baseline
  bool exceeds(double currentAvgMs) const {
    double tolerance = avgMs * (tolerancePercent / 100.0);
    return currentAvgMs > (avgMs + tolerance);
  }
};

class PerformanceBaseline {
public:
  // Load baseline from JSON file
  static bool load(const std::string& path);

  // Check if current metric exceeds baseline
  static bool exceedsBaseline(const std::string& name, double currentAvgMs);

  // Get baseline for metric name
  static const BaselineMetric* getBaseline(const std::string& name);

  // Print comparison report
  static void printComparisonReport(const std::string& name, double currentAvgMs);

private:
  static std::unordered_map<std::string, BaselineMetric> baselines_;
  static bool parseJsonLine(const std::string& line, std::string& key, double& value);
};

}  // namespace milkdrop
