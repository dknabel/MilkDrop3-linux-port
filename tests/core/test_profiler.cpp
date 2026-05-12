#include <gtest/gtest.h>
#include "core/profiler.h"
#include <thread>
#include <chrono>

TEST(ProfilerTest, TimingSectionBasic) {
  auto& prof = milkdrop::Profiler::instance();
  prof.reset();

  prof.startSection("test");
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  prof.endSection("test");

  auto metrics = prof.getMetrics("test");
  ASSERT_NE(metrics, nullptr);
  EXPECT_GE(metrics->avgMs(), 9.0); // At least 9ms
  EXPECT_EQ(metrics->callCount, 1);
}

TEST(ProfilerTest, RAIITimer) {
  auto& prof = milkdrop::Profiler::instance();
  prof.reset();

  {
    milkdrop::SectionTimer timer("raii_test");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  auto metrics = prof.getMetrics("raii_test");
  ASSERT_NE(metrics, nullptr);
  EXPECT_GE(metrics->avgMs(), 4.0);
}
