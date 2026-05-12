#include "performance_baseline.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace milkdrop {

std::unordered_map<std::string, BaselineMetric> PerformanceBaseline::baselines_;

bool PerformanceBaseline::parseJsonLine(const std::string& line, std::string& key, double& value) {
  // Simple manual parsing for JSON lines like: "key": number
  size_t quoteStart = line.find('"');
  if (quoteStart == std::string::npos) {
    return false;
  }

  size_t quoteEnd = line.find('"', quoteStart + 1);
  if (quoteEnd == std::string::npos) {
    return false;
  }

  key = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

  // Find the colon and number
  size_t colonPos = line.find(':', quoteEnd);
  if (colonPos == std::string::npos) {
    return false;
  }

  // Extract the number part
  size_t numStart = line.find_first_of("0123456789-.", colonPos);
  if (numStart == std::string::npos) {
    return false;
  }

  size_t numEnd = numStart;
  while (numEnd < line.length() && (std::isdigit(line[numEnd]) || line[numEnd] == '.' || line[numEnd] == '-')) {
    numEnd++;
  }

  try {
    value = std::stod(line.substr(numStart, numEnd - numStart));
    return true;
  } catch (...) {
    return false;
  }
}

bool PerformanceBaseline::load(const std::string& path) {
  std::ifstream file(path);
  if (!file) {
    std::cerr << "Failed to open baseline file: " << path << "\n";
    return false;
  }

  std::string line;
  std::string currentMetric;
  BaselineMetric currentBaseline;
  int metricsLoaded = 0;

  while (std::getline(file, line)) {
    // Skip empty lines and comments
    if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
      continue;
    }

    // Check if this is a metric name line (quoted string followed by {)
    if (line.find("\"") != std::string::npos && line.find("{") != std::string::npos) {
      // Extract metric name
      size_t start = line.find("\"") + 1;
      size_t end = line.find("\"", start);
      if (start != std::string::npos && end != std::string::npos) {
        // If we have a previous metric, save it
        if (!currentMetric.empty()) {
          baselines_[currentMetric] = currentBaseline;
          metricsLoaded++;
          currentBaseline = BaselineMetric();
        }
        currentMetric = line.substr(start, end - start);
      }
      continue;
    }

    // Try to parse key-value pairs
    std::string key;
    double value;
    if (parseJsonLine(line, key, value)) {
      if (key == "avgMs") {
        currentBaseline.avgMs = value;
      } else if (key == "maxMs") {
        currentBaseline.maxMs = value;
      } else if (key == "tolerancePercent") {
        currentBaseline.tolerancePercent = value;
      }
    }
  }

  // Don't forget the last metric
  if (!currentMetric.empty()) {
    baselines_[currentMetric] = currentBaseline;
    metricsLoaded++;
  }

  file.close();

  if (metricsLoaded == 0) {
    std::cerr << "Failed to parse any baseline metrics from: " << path << "\n";
    return false;
  }

  std::cout << "Loaded " << metricsLoaded << " baseline metrics\n";
  return true;
}

bool PerformanceBaseline::exceedsBaseline(const std::string& name, double currentAvgMs) {
  auto it = baselines_.find(name);
  if (it == baselines_.end()) {
    std::cerr << "No baseline for metric: " << name << "\n";
    return false;
  }

  return it->second.exceeds(currentAvgMs);
}

const BaselineMetric* PerformanceBaseline::getBaseline(const std::string& name) {
  auto it = baselines_.find(name);
  return it != baselines_.end() ? &it->second : nullptr;
}

void PerformanceBaseline::printComparisonReport(const std::string& name, double currentAvgMs) {
  auto baseline = getBaseline(name);
  if (!baseline) {
    std::cout << "  No baseline for: " << name << "\n";
    return;
  }

  double tolerance = baseline->avgMs * (baseline->tolerancePercent / 100.0);
  double threshold = baseline->avgMs + tolerance;
  bool exceeds = currentAvgMs > threshold;

  std::cout << "  " << name << ":\n";
  std::cout << "    Baseline: " << std::fixed << std::setprecision(4) << baseline->avgMs << " ms\n";
  std::cout << "    Current:  " << currentAvgMs << " ms\n";
  std::cout << "    Tolerance: " << baseline->tolerancePercent << "%\n";
  std::cout << "    Threshold: " << threshold << " ms\n";
  std::cout << "    Status: " << (exceeds ? "REGRESSION" : "OK") << "\n";
}

}  // namespace milkdrop
