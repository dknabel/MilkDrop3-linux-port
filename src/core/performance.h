#pragma once
#include <chrono>
#include <string>

namespace milkdrop {

struct PerformanceMetrics {
  std::string name;
  double totalMs = 0.0;
  double minMs = 1e9;
  double maxMs = 0.0;
  int callCount = 0;

  double avgMs() const { return callCount > 0 ? totalMs / callCount : 0.0; }
};

constexpr double TARGET_FRAME_TIME_MS = 16.67; // 60 FPS
constexpr double FRAME_TIME_BUDGET_MS = 16.0;
constexpr double EVAL_TIME_BUDGET_MS = 8.0;   // Per-frame equation eval budget
constexpr double RENDER_TIME_BUDGET_MS = 8.0;  // Render command generation budget

} // namespace milkdrop
