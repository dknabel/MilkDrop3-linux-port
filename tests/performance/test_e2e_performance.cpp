#include <gtest/gtest.h>
#include "core/visualization.h"
#include "core/profiler.h"
#include <iostream>

TEST(EndToEndPerformanceTest, FullRenderingPipeline60FPS) {
  auto& prof = milkdrop::Profiler::instance();
  prof.reset();

  VisualizationEngine engine;

  std::string presetContent = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time);
rotation = time * 0.5;
decay = 0.95;

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index) * 0.5;

[shape_0_init]
x = 0.5;
y = 0.5;
radius = 0.1;

[shape_0_per_frame]
x = 0.5 + 0.1 * sin(time);
y = 0.5 + 0.1 * cos(time);
radius = 0.1 + 0.05 * sin(time * 2.0);
)";

  engine.loadPreset(presetContent, "e2e_test.milk");

  std::vector<float> freqBins(64);

  // Simulate 300 frames at 60 FPS (5 seconds)
  double totalFrameTime = 0.0;
  int frameCount = 300;

  for (int frame = 0; frame < frameCount; ++frame) {
    // Generate random-ish frequency data
    for (int i = 0; i < 64; ++i) {
      freqBins[i] = 0.3f + 0.2f * std::sin(frame * 0.1f + i * 0.1f);
    }

    {
      milkdrop::SectionTimer timer("e2e_frame");
      engine.update(freqBins, 0.016f);
      auto commands = engine.getRenderCommands();
    }
  }

  auto metrics = prof.getMetrics("e2e_frame");
  ASSERT_NE(metrics, nullptr);

  double avgFrameTime = metrics->avgMs();
  double budgetMs = milkdrop::FRAME_TIME_BUDGET_MS;
  double fps = 1000.0 / avgFrameTime;

  std::cout << "\n=== End-to-End Performance Report ===\n";
  std::cout << "Frames: " << frameCount << "\n";
  std::cout << "Avg frame time: " << avgFrameTime << " ms\n";
  std::cout << "Estimated FPS: " << fps << "\n";
  std::cout << "Budget: " << budgetMs << " ms (60 FPS)\n";
  std::cout << "Status: " << (avgFrameTime <= budgetMs ? "PASS" : "FAIL") << "\n";

  // Must achieve 60 FPS (16ms per frame)
  EXPECT_LE(avgFrameTime, budgetMs);
}

TEST(EndToEndPerformanceTest, MultiPresetSequence) {
  auto& prof = milkdrop::Profiler::instance();
  prof.reset();

  VisualizationEngine engine;

  // Test sequence of different presets
  std::vector<std::string> presets = {
    // Preset 1: Simple
    R"([per_frame_eqs_0]
zoom = 1.0 + 0.05 * sin(time);
[wave_0_init]
y = 0.0;
[wave_0_per_point]
y = sample(index) * 0.5;)",

    // Preset 2: More complex
    R"([per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time);
rotation = time * 0.3;
[wave_0_init]
y = 0.0;
[wave_0_per_point]
y = sample(index) * 0.4;
[shape_0_init]
x = 0.5;
[shape_0_per_frame]
x = 0.5 + 0.1 * sin(time);)",

    // Preset 3: Very complex
    R"([per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time) + 0.05 * cos(time * 2.0);
rotation = time * 0.5;
[wave_0_init]
y = 0.0;
[wave_0_per_point]
y = sample(index) * 0.5;
[wave_1_init]
y = 0.5;
[wave_1_per_point]
y = 0.5 + sample(index) * 0.3;
[shape_0_init]
x = 0.5;
[shape_0_per_frame]
x = 0.5 + 0.15 * sin(time);)"
  };

  std::vector<float> freqBins(64, 0.3f);
  int totalFrames = 0;

  for (size_t p = 0; p < presets.size(); ++p) {
    engine.loadPreset(presets[p], "preset_" + std::to_string(p) + ".milk");

    // Run 100 frames per preset
    for (int frame = 0; frame < 100; ++frame) {
      {
        milkdrop::SectionTimer timer("e2e_multi_preset");
        engine.update(freqBins, 0.016f);
        auto commands = engine.getRenderCommands();
      }
      totalFrames++;
    }
  }

  auto metrics = prof.getMetrics("e2e_multi_preset");
  ASSERT_NE(metrics, nullptr);

  double avgFrameTime = metrics->avgMs();
  double budgetMs = milkdrop::FRAME_TIME_BUDGET_MS;

  std::cout << "\n=== Multi-Preset Sequence Report ===\n";
  std::cout << "Total frames: " << totalFrames << "\n";
  std::cout << "Avg frame time: " << avgFrameTime << " ms\n";
  std::cout << "Budget: " << budgetMs << " ms (60 FPS)\n";
  std::cout << "Status: " << (avgFrameTime <= budgetMs ? "PASS" : "FAIL") << "\n";

  EXPECT_LE(avgFrameTime, budgetMs);
}

TEST(EndToEndPerformanceTest, SustainedLoad) {
  auto& prof = milkdrop::Profiler::instance();
  prof.reset();

  VisualizationEngine engine;

  std::string complexPreset = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time) + 0.05 * cos(time * 2.0);
rotation = time * 0.5 + 0.3 * sin(time * 0.3);
decay = 0.92 + 0.05 * sin(time);
center_x = 0.5 + 0.1 * sin(time * 0.7);
center_y = 0.5 + 0.1 * cos(time * 0.7);

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index) * 0.4;

[wave_1_init]
y = 0.5;

[wave_1_per_point]
y = 0.5 + sample(index) * 0.3;

[shape_0_init]
x = 0.5;
y = 0.5;
radius = 0.1;

[shape_0_per_frame]
x = 0.5 + 0.15 * sin(time);
y = 0.5 + 0.15 * cos(time);
radius = 0.1 + 0.08 * sin(time * 1.5);

[shape_1_init]
x = 0.3;
y = 0.3;

[shape_1_per_frame]
x = 0.3 + 0.1 * cos(time * 0.7);
y = 0.3 + 0.1 * sin(time * 0.7);
)";

  engine.loadPreset(complexPreset, "sustained_load.milk");

  std::vector<float> freqBins(64);

  // Simulate 10 seconds at 60 FPS = 600 frames
  for (int frame = 0; frame < 600; ++frame) {
    // Vary frequency data throughout
    for (int i = 0; i < 64; ++i) {
      freqBins[i] = 0.3f + 0.2f * std::sin(frame * 0.02f + i * 0.1f);
    }

    {
      milkdrop::SectionTimer timer("e2e_sustained");
      engine.update(freqBins, 0.016f);
      auto commands = engine.getRenderCommands();
    }
  }

  auto metrics = prof.getMetrics("e2e_sustained");
  ASSERT_NE(metrics, nullptr);

  double avgFrameTime = metrics->avgMs();
  double maxFrameTime = metrics->maxMs;
  double budgetMs = milkdrop::FRAME_TIME_BUDGET_MS;

  std::cout << "\n=== Sustained Load Report (10 seconds) ===\n";
  std::cout << "Frames: 600\n";
  std::cout << "Avg frame time: " << avgFrameTime << " ms\n";
  std::cout << "Max frame time: " << maxFrameTime << " ms\n";
  std::cout << "Budget: " << budgetMs << " ms (60 FPS)\n";
  std::cout << "Status: " << (avgFrameTime <= budgetMs ? "PASS" : "FAIL") << "\n";

  EXPECT_LE(avgFrameTime, budgetMs);
  EXPECT_LE(maxFrameTime, 20.0); // Occasional frame can exceed budget but not by much
}
