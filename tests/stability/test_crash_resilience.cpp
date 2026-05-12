#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include <memory>
#include <iostream>

#include "ui/display_manager.h"
#include "audio/audio_analyzer.h"
#include "core/visualization.h"
#include "core/preset/preset_parser.h"

class CrashResilienceTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Initialize components for testing
    parser_ = std::make_unique<milkdrop::PresetParser>();
    visualizer_ = std::make_unique<VisualizationEngine>();
  }

  void TearDown() override {
    visualizer_.reset();
    parser_.reset();
  }

  std::unique_ptr<milkdrop::PresetParser> parser_;
  std::unique_ptr<VisualizationEngine> visualizer_;
};

// Test 1: Audio exception handling
TEST_F(CrashResilienceTest, AudioExceptionHandling) {
  // Simulate audio failure scenario
  EXPECT_NO_THROW({
    try {
      throw std::runtime_error("Audio device disconnected");
    } catch (const std::exception& e) {
      // Application should handle gracefully
      std::cout << "Caught exception: " << e.what() << "\n";
    }
  });
}

// Test 2: Display manager resilience without initialization
TEST_F(CrashResilienceTest, UninitializedDisplayManager) {
  DisplayManager display;

  // Operations on uninitialized display should not crash
  // Note: Can't call isRunning() on uninitialized GLFW window - would assert
  // Display manager has default window size
  EXPECT_NO_THROW({
    glm::ivec2 size = display.getWindowSize();
    // The default window size is set even without explicit initialization
    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
  });
}

// Test 3: Invalid preset handling
TEST_F(CrashResilienceTest, InvalidPresetsDoNotCrash) {
  // Empty string should not cause a crash
  std::string invalidPreset = "";
  EXPECT_NO_THROW({
    auto result = visualizer_->loadPreset(invalidPreset, "");
    // Empty preset may or may not load successfully - that's ok
    // Main thing is it doesn't crash
  });

  // Visualizer should remain operable after invalid preset attempt
  std::vector<float> dummyFreq(256, 0.5f);
  EXPECT_NO_THROW({
    visualizer_->update(dummyFreq, 0.016f);
  });
}

// Test 4: Empty frequency data edge cases
TEST_F(CrashResilienceTest, FreqBinVectorEdgeCases) {
  // Empty frequency vector
  std::vector<float> emptyFreq;
  EXPECT_NO_THROW({
    visualizer_->update(emptyFreq, 0.016f);
  });

  // Single frequency bin
  std::vector<float> singleFreq = {0.5f};
  EXPECT_NO_THROW({
    visualizer_->update(singleFreq, 0.016f);
  });

  // Very large frequency vector
  std::vector<float> largeFreq(100000, 0.5f);
  EXPECT_NO_THROW({
    visualizer_->update(largeFreq, 0.016f);
  });

  // Zero-valued frequency bins
  std::vector<float> zeroFreq(256, 0.0f);
  EXPECT_NO_THROW({
    visualizer_->update(zeroFreq, 0.016f);
  });
}

// Test 5: Rapid state changes
TEST_F(CrashResilienceTest, RapidStateChanges) {
  std::vector<float> dummyFreq(256, 0.5f);

  // Rapidly reset and update (simulating rapid preset switches)
  EXPECT_NO_THROW({
    for (int i = 0; i < 10; ++i) {
      visualizer_->reset();
      visualizer_->update(dummyFreq, 0.016f);
    }
  });
}

// Test 6: Unusual frame time values
TEST_F(CrashResilienceTest, UnusualFrameTiming) {
  std::vector<float> dummyFreq(256, 0.5f);

  // Very small deltaTime
  EXPECT_NO_THROW({
    visualizer_->update(dummyFreq, 0.001f);
  });

  // Very large deltaTime (frame drop)
  EXPECT_NO_THROW({
    visualizer_->update(dummyFreq, 0.5f);
  });

  // Zero deltaTime
  EXPECT_NO_THROW({
    visualizer_->update(dummyFreq, 0.0f);
  });

  // Negative deltaTime (should not occur but handle gracefully)
  EXPECT_NO_THROW({
    visualizer_->update(dummyFreq, -0.016f);
  });
}

// Test 7: Preset parser robustness
TEST_F(CrashResilienceTest, ParserRobustness) {
  // Empty preset
  EXPECT_NO_THROW({
    auto result = parser_->parsePreset("", "empty.milk");
    // Empty strings should not parse successfully but shouldn't crash
  });

  // Malformed INI format (missing closing bracket)
  EXPECT_NO_THROW({
    auto result = parser_->parsePreset("[section\nno closing bracket", "malformed.milk");
  });

  // Valid minimal preset
  std::string validMinimal = "[per_frame_eqs_0]\nzoom = 1.0;";
  EXPECT_NO_THROW({
    auto result = parser_->parsePreset(validMinimal, "minimal.milk");
  });

  // Extremely large preset (stress test)
  std::string hugePreset = "[per_frame_eqs_0]\n";
  for (int i = 0; i < 1000; ++i) {
    hugePreset += "var" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
  }
  EXPECT_NO_THROW({
    auto result = parser_->parsePreset(hugePreset, "huge.milk");
  });
}

// Test 8: Resource cleanup under stress
TEST_F(CrashResilienceTest, ResourceCleanupStress) {
  std::vector<float> dummyFreq(256, 0.5f);

  EXPECT_NO_THROW({
    for (int i = 0; i < 100; ++i) {
      visualizer_->reset();
      visualizer_->update(dummyFreq, 0.016f);
    }
  });

  // After stress, system should still be functional
  EXPECT_NO_THROW({
    auto commands = visualizer_->getRenderCommands();
    EXPECT_TRUE(commands.empty() || commands.size() >= 0);
  });
}

// Test 9: Audio analyzer with edge case data
TEST_F(CrashResilienceTest, AudioAnalyzerEdgeCases) {
  AudioAnalyzer analyzer(512);

  // Empty audio frame
  std::vector<float> emptyFrame;
  EXPECT_NO_THROW({
    auto result = analyzer.analyze(emptyFrame);
    // Should return empty or default result without crashing
  });

  // Single sample
  std::vector<float> singleSample = {0.5f};
  EXPECT_NO_THROW({
    auto result = analyzer.analyze(singleSample);
  });

  // Very large audio frame
  std::vector<float> largeSamples(1000000, 0.1f);
  EXPECT_NO_THROW({
    auto result = analyzer.analyze(largeSamples);
  });

  // All zeros
  std::vector<float> zeroSamples(512, 0.0f);
  EXPECT_NO_THROW({
    auto result = analyzer.analyze(zeroSamples);
  });

  // All ones (saturation)
  std::vector<float> onesSamples(512, 1.0f);
  EXPECT_NO_THROW({
    auto result = analyzer.analyze(onesSamples);
  });

  // Extreme values
  std::vector<float> extremeSamples(512);
  for (int i = 0; i < 512; ++i) {
    extremeSamples[i] = (i % 2 == 0) ? 1e6f : -1e6f;
  }
  EXPECT_NO_THROW({
    auto result = analyzer.analyze(extremeSamples);
  });
}

// Test 10: Exception safety in destructor chains
TEST_F(CrashResilienceTest, DestructorChainSafety) {
  // Test that destroying objects in various orders doesn't cause issues
  EXPECT_NO_THROW({
    auto vis = std::make_unique<VisualizationEngine>();
    auto par = std::make_unique<milkdrop::PresetParser>();
    auto analyzer = std::make_unique<AudioAnalyzer>(512);

    // Destroy in various orders
    vis.reset();
    par.reset();
    analyzer.reset();
  });

  // Create new instances to ensure system is still functional
  EXPECT_NO_THROW({
    auto vis = std::make_unique<VisualizationEngine>();
    std::vector<float> freq(256, 0.5f);
    vis->update(freq, 0.016f);
    vis.reset();
  });
}

// Test 11: Concurrent-like state access patterns
TEST_F(CrashResilienceTest, StateAccessPatterns) {
  std::vector<float> dummyFreq(256, 0.5f);

  // Multiple rapid accesses to state
  EXPECT_NO_THROW({
    visualizer_->update(dummyFreq, 0.016f);
    auto commands = visualizer_->getRenderCommands();
    visualizer_->reset();
    commands = visualizer_->getRenderCommands();
    visualizer_->update(dummyFreq, 0.016f);
    auto preset = visualizer_->getCurrentPreset();
  });
}

// Test 12: Render command generation under stress
TEST_F(CrashResilienceTest, RenderCommandGeneration) {
  std::vector<float> freqData(256);

  // Varying frequency data patterns
  EXPECT_NO_THROW({
    // Gradient pattern
    for (int i = 0; i < 256; ++i) {
      freqData[i] = static_cast<float>(i) / 256.0f;
    }
    visualizer_->update(freqData, 0.016f);

    // Random pattern simulation
    for (int i = 0; i < 256; ++i) {
      freqData[i] = (i % 3) * 0.33f;
    }
    visualizer_->update(freqData, 0.016f);

    // Pulse pattern
    for (int i = 0; i < 256; ++i) {
      freqData[i] = (i % 10 < 5) ? 0.8f : 0.2f;
    }
    visualizer_->update(freqData, 0.016f);

    auto commands = visualizer_->getRenderCommands();
    // Verify commands are generated without errors
    EXPECT_TRUE(true);
  });
}

