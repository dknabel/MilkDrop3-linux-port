#include "profiler.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace milkdrop {

Profiler& Profiler::instance() {
  static Profiler prof;
  return prof;
}

void Profiler::startSection(const std::string& name) {
  activeTimings_[name].start = std::chrono::high_resolution_clock::now();
}

void Profiler::endSection(const std::string& name) {
  auto it = activeTimings_.find(name);
  if (it == activeTimings_.end()) return;

  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration<double, std::milli>(end - it->second.start).count();

  auto& metric = metrics_[name];
  metric.name = name;
  metric.totalMs += elapsed;
  metric.minMs = std::min(metric.minMs, elapsed);
  metric.maxMs = std::max(metric.maxMs, elapsed);
  metric.callCount++;

  activeTimings_.erase(it);
}

const PerformanceMetrics* Profiler::getMetrics(const std::string& name) const {
  auto it = metrics_.find(name);
  return it != metrics_.end() ? &it->second : nullptr;
}

void Profiler::printReport() const {
  std::cout << "\n=== Performance Report ===\n";
  std::cout << std::left << std::setw(30) << "Section"
            << std::right << std::setw(12) << "Calls"
            << std::setw(12) << "Total(ms)"
            << std::setw(12) << "Avg(ms)"
            << std::setw(12) << "Min(ms)"
            << std::setw(12) << "Max(ms)" << "\n";
  std::cout << std::string(90, '-') << "\n";

  for (const auto& [name, metric] : metrics_) {
    std::cout << std::left << std::setw(30) << name
              << std::right << std::setw(12) << metric.callCount
              << std::setw(12) << std::fixed << std::setprecision(3) << metric.totalMs
              << std::setw(12) << metric.avgMs()
              << std::setw(12) << metric.minMs
              << std::setw(12) << metric.maxMs << "\n";
  }
}

bool Profiler::exceededBudget(const std::string& name, double budgetMs) const {
  auto it = metrics_.find(name);
  return it != metrics_.end() && it->second.avgMs() > budgetMs;
}

void Profiler::reset() {
  metrics_.clear();
  activeTimings_.clear();
}

SectionTimer::SectionTimer(const std::string& name) : name_(name) {
  Profiler::instance().startSection(name);
}

SectionTimer::~SectionTimer() {
  Profiler::instance().endSection(name_);
}

} // namespace milkdrop
