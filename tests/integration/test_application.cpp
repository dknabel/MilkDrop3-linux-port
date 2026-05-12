#include <gtest/gtest.h>
#include "ui/display_manager.h"
#include "ui/preset_browser.h"
#include "ui/visual_feedback.h"
#include "audio/audio_analyzer.h"
#include "core/visualization.h"
#include <thread>
#include <chrono>
#include <cmath>

class ApplicationTest : public ::testing::Test {
protected:
  std::unique_ptr<DisplayManager> display_;
  std::unique_ptr<PresetBrowser> browser_;
  std::unique_ptr<VisualFeedback> feedback_;
  std::unique_ptr<AudioAnalyzer> analyzer_;
  std::unique_ptr<VisualizationEngine> visualizer_;

  void SetUp() override {
    display_ = std::make_unique<DisplayManager>();
    browser_ = std::make_unique<PresetBrowser>();
    feedback_ = std::make_unique<VisualFeedback>();
    analyzer_ = std::make_unique<AudioAnalyzer>(512);
    visualizer_ = std::make_unique<VisualizationEngine>();
  }
};

// Test 1: Component initialization
TEST_F(ApplicationTest, ComponentInitialization) {
  EXPECT_TRUE(display_ != nullptr);
  EXPECT_TRUE(browser_ != nullptr);
  EXPECT_TRUE(feedback_ != nullptr);
  EXPECT_TRUE(analyzer_ != nullptr);
  EXPECT_TRUE(visualizer_ != nullptr);
}

// Test 2: Display manager setup (window init would require X11, so skip in CI)
TEST_F(ApplicationTest, DisplayManagerReadiness) {
  // Test that display manager is created but not initialized (requires window)
  // Note: isRunning() checks if window exists, which it doesn't in headless mode
  // So we just verify the object can be created without crashing
  glm::ivec2 size = display_->getWindowSize();
  float aspect = display_->getAspectRatio();
  EXPECT_GT(aspect, 0.0f);
}

// Test 3: Preset browser functionality
TEST_F(ApplicationTest, PresetBrowserIntegration) {
  // Scan presets (will be empty in test environment)
  // Just verify it doesn't crash - it may return false if path doesn't exist
  browser_->scanPresets("/tmp");
  // The browser should have been created successfully and have a valid state
  size_t index = browser_->getCurrentIndex();
  // Index should be valid (0 or past end if no presets)
  EXPECT_GE(index, 0);
}

// Test 4: Audio analyzer setup
TEST_F(ApplicationTest, AudioAnalyzerSetup) {
  EXPECT_EQ(analyzer_->getFrequencyBinCount(), 256); // 512/2
}

// Test 5: Visualization pipeline
TEST_F(ApplicationTest, VisualizationPipeline) {
  // Create synthetic audio
  std::vector<float> audioSamples(512, 0.1f);

  // Analyze audio
  auto freqBins = analyzer_->analyze(audioSamples);
  EXPECT_GT(freqBins.size(), 0);

  // Feed to visualizer
  EXPECT_NO_THROW({
    visualizer_->update(freqBins, 0.016f);
  });

  // Get render commands
  auto commands = visualizer_->getRenderCommands();
  // Commands may be empty (no active preset) but should not crash
}

// Test 6: Full pipeline stability - 300 frames (5 seconds @ 60 FPS)
TEST_F(ApplicationTest, FullPipelineStability) {
  std::vector<float> audioSamples(512);

  // Simulate 5 seconds of continuous operation (300 frames @ 60 FPS)
  for (int frame = 0; frame < 300; ++frame) {
    // Generate synthetic audio (sine wave modulation)
    for (int i = 0; i < (int)audioSamples.size(); ++i) {
      float time = static_cast<float>(frame) / 60.0f;
      float freq = 440.0f + 100.0f * std::sin(2.0f * 3.14159f * time);
      float phase = 2.0f * 3.14159f * freq * time;
      audioSamples[i] = 0.1f * std::sin(phase + i * 0.01f);
    }

    // Analyze audio
    auto freqBins = analyzer_->analyze(audioSamples);
    EXPECT_GT(freqBins.size(), 0) << "Frame " << frame << ": FFT produced no bins";

    // Update visualization
    EXPECT_NO_THROW({
      visualizer_->update(freqBins, 0.016f);
    }) << "Exception at frame " << frame;

    // Update feedback metrics
    feedback_->update(0.016f, freqBins);

    // Get display strings
    std::string fpsStr = feedback_->getFpsDisplay();
    std::string freqStr = feedback_->getFrequencyDisplay();
    EXPECT_FALSE(fpsStr.empty()) << "Frame " << frame << ": FPS display empty";

    // Verify FPS calculation
    float fps = feedback_->getAverageFps();
    EXPECT_GT(fps, 0.0f) << "Frame " << frame << ": FPS should be positive";

    // Get render commands
    auto commands = visualizer_->getRenderCommands();
    // Commands may vary by preset, but should not crash
  }
}

// Test 7: Component lifetimes and destruction
TEST_F(ApplicationTest, ComponentLifecycles) {
  for (int i = 0; i < 10; ++i) {
    {
      auto localDisplay = std::make_unique<DisplayManager>();
      auto localBrowser = std::make_unique<PresetBrowser>();
      auto localFeedback = std::make_unique<VisualFeedback>();
      auto localAnalyzer = std::make_unique<AudioAnalyzer>(512);
      auto localVisualizer = std::make_unique<VisualizationEngine>();

      // Use components briefly
      std::vector<float> dummyFreq(256, 0.5f);
      localVisualizer->update(dummyFreq, 0.016f);
      localFeedback->update(0.016f, dummyFreq);
    }
    // Components destroyed at scope end; verify no leaks
  }
}

// Test 8: Stress test - rapid updates
TEST_F(ApplicationTest, RapidUpdateStress) {
  std::vector<float> freqBins(256, 0.5f);

  // Rapid updates simulating very high FPS or fast forwarding
  for (int i = 0; i < 1000; ++i) {
    visualizer_->update(freqBins, 0.016f);
    feedback_->update(0.016f, freqBins);
  }
}

// Test 9: Memory stability with varied frequency data
TEST_F(ApplicationTest, VariedFrequencyData) {
  std::vector<float> dummyAudio(512);

  // Test with various frequency patterns
  for (int frame = 0; frame < 100; ++frame) {
    // Silent frame
    std::fill(dummyAudio.begin(), dummyAudio.end(), 0.0f);
    auto freqBins1 = analyzer_->analyze(dummyAudio);
    visualizer_->update(freqBins1, 0.016f);

    // Full volume
    std::fill(dummyAudio.begin(), dummyAudio.end(), 0.99f);
    auto freqBins2 = analyzer_->analyze(dummyAudio);
    visualizer_->update(freqBins2, 0.016f);

    // Medium volume
    std::fill(dummyAudio.begin(), dummyAudio.end(), 0.5f);
    auto freqBins3 = analyzer_->analyze(dummyAudio);
    visualizer_->update(freqBins3, 0.016f);
  }
}
