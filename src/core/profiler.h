#pragma once
#include "performance.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace milkdrop {

class Profiler {
public:
  static Profiler& instance();

  // Start timing a named section
  void startSection(const std::string& name);

  // End timing a named section
  void endSection(const std::string& name);

  // Get metrics for a section
  const PerformanceMetrics* getMetrics(const std::string& name) const;

  // Print all metrics to stdout
  void printReport() const;

  // Reset all metrics
  void reset();

  // Check if section exceeded budget
  bool exceededBudget(const std::string& name, double budgetMs) const;

private:
  Profiler() = default;

  struct TimingContext {
    std::chrono::high_resolution_clock::time_point start;
  };

  std::unordered_map<std::string, PerformanceMetrics> metrics_;
  std::unordered_map<std::string, TimingContext> activeTimings_;
};

// RAII helper for automatic timing
class SectionTimer {
public:
  explicit SectionTimer(const std::string& name);
  ~SectionTimer();

private:
  std::string name_;
};

} // namespace milkdrop
