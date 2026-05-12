#include <gtest/gtest.h>
#include "core/preset/preset_parser.h"
#include "core/expression/expression_evaluator.h"
#include "core/visualization.h"
#include <filesystem>
#include <vector>
#include <fstream>

namespace fs = std::filesystem;

namespace milkdrop {

class RealPresetTest : public ::testing::Test {
protected:
  PresetParser parser_;
  ExpressionEvaluator evaluator_;
  VisualizationEngine visualizer_;

  std::vector<std::string> findTestPresets() {
    std::vector<std::string> presets;
    std::vector<fs::path> searchPaths = {
      fs::path("tests/fixtures/presets"),
      fs::path("tests/presets"),
      fs::path("presets")
    };

    for (const auto& testPresetsDir : searchPaths) {
      if (fs::exists(testPresetsDir)) {
        for (const auto& entry : fs::recursive_directory_iterator(testPresetsDir)) {
          if (entry.path().extension() == ".milk") {
            presets.push_back(entry.path().string());
          }
        }
      }
    }

    return presets;
  }

  std::string readPresetFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  }
};

// Test 1: Parse valid presets
TEST_F(RealPresetTest, ParseValidPresets) {
  auto presets = findTestPresets();

  if (presets.empty()) {
    GTEST_SKIP() << "No test presets found in fixtures";
  }

  for (const auto& presetPath : presets) {
    std::string content = readPresetFile(presetPath);
    EXPECT_FALSE(content.empty()) << "Cannot read preset: " << presetPath;

    std::string filename = fs::path(presetPath).filename().string();
    auto parseResult = parser_.parsePreset(content, filename);
    EXPECT_TRUE(parseResult.has_value()) << "Failed to parse: " << presetPath;

    if (parseResult.has_value()) {
      EXPECT_EQ(parseResult->filename, filename);
    }
  }
}

// Test 2: Evaluate per-frame expressions for real presets
TEST_F(RealPresetTest, EvaluatePerFrameExpressions) {
  auto presets = findTestPresets();

  if (presets.empty()) {
    GTEST_SKIP() << "No test presets found in fixtures";
  }

  for (const auto& presetPath : presets) {
    std::string content = readPresetFile(presetPath);
    EXPECT_FALSE(content.empty()) << "Cannot read preset: " << presetPath;

    std::string filename = fs::path(presetPath).filename().string();
    auto parseResult = parser_.parsePreset(content, filename);

    if (!parseResult.has_value()) {
      GTEST_SKIP() << "Parse failed for: " << presetPath;
    }

    // Try to load and update with dummy frequency data
    std::vector<float> dummyFreq(256, 0.5f);
    EXPECT_NO_THROW({
      visualizer_.loadPreset(content, filename);
      visualizer_.update(dummyFreq, 0.016f);
    }) << "Exception during evaluation for: " << presetPath;
  }
}

// Test 3: Handle corrupted/invalid presets gracefully
TEST_F(RealPresetTest, HandlesCorruptedPresets) {
  std::string invalidPreset = "[invalid]\ngarbled data!@#$%\n";

  // Parser should not crash
  EXPECT_NO_THROW({
    auto result = parser_.parsePreset(invalidPreset, "corrupted.milk");
    // Result may be invalid but should not throw
  });
}

// Test 4: Multi-frame execution stability
TEST_F(RealPresetTest, MultiFrameExecution) {
  auto presets = findTestPresets();

  if (presets.empty()) {
    GTEST_SKIP() << "No test presets found in fixtures";
  }

  // Use first preset for multi-frame test
  std::string content = readPresetFile(presets[0]);
  std::string filename = fs::path(presets[0]).filename().string();

  auto parseResult = parser_.parsePreset(content, filename);
  if (!parseResult.has_value()) {
    GTEST_SKIP() << "Parse failed for: " << presets[0];
  }

  // Load preset
  bool loaded = visualizer_.loadPreset(content, filename);
  if (!loaded) {
    GTEST_SKIP() << "Failed to load preset: " << presets[0];
  }

  // Run 300 frames of evaluation
  std::vector<float> dummyFreq(256, 0.3f);
  for (int i = 0; i < 300; ++i) {
    EXPECT_NO_THROW({
      visualizer_.update(dummyFreq, 0.016f);
    }) << "Crash on frame " << i << " for preset: " << presets[0];
  }
}

// Test 5: Sequential preset loading and switching
TEST_F(RealPresetTest, SequentialPresetLoading) {
  auto presets = findTestPresets();

  if (presets.size() < 2) {
    GTEST_SKIP() << "Need at least 2 presets for sequential loading test";
  }

  std::vector<float> dummyFreq(256, 0.5f);

  // Load first preset, run 100 frames
  {
    std::string content = readPresetFile(presets[0]);
    std::string filename = fs::path(presets[0]).filename().string();

    auto parseResult = parser_.parsePreset(content, filename);
    if (!parseResult.has_value()) {
      GTEST_SKIP() << "Failed to parse first preset";
    }

    EXPECT_TRUE(visualizer_.loadPreset(content, filename));

    for (int i = 0; i < 100; ++i) {
      EXPECT_NO_THROW({
        visualizer_.update(dummyFreq, 0.016f);
      }) << "Error on frame " << i << " of first preset";
    }
  }

  // Load second preset, run 100 frames
  {
    std::string content = readPresetFile(presets[1]);
    std::string filename = fs::path(presets[1]).filename().string();

    auto parseResult = parser_.parsePreset(content, filename);
    if (!parseResult.has_value()) {
      GTEST_SKIP() << "Failed to parse second preset";
    }

    EXPECT_TRUE(visualizer_.loadPreset(content, filename));

    for (int i = 0; i < 100; ++i) {
      EXPECT_NO_THROW({
        visualizer_.update(dummyFreq, 0.016f);
      }) << "Error on frame " << i << " of second preset";
    }
  }
}

// Test 6: Preset with complex expressions
TEST_F(RealPresetTest, PresetWithComplexExpressions) {
  std::string complexPreset = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time);
rotation = time * 0.5;
cx = 0.5 + 0.1 * cos(time);
cy = 0.5 + 0.1 * sin(time);

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index) * 0.5;

[shape_0_init]
x = 0.5;
y = 0.5;
radius = 0.1;

[shape_0_per_frame]
radius = 0.1 + 0.05 * sin(time);
)";

  auto parseResult = parser_.parsePreset(complexPreset, "complex.milk");
  EXPECT_TRUE(parseResult.has_value());

  if (parseResult.has_value()) {
    // Try to load and run multiple frames
    std::vector<float> dummyFreq(256, 0.5f);

    EXPECT_NO_THROW({
      visualizer_.loadPreset(complexPreset, "complex.milk");
      for (int i = 0; i < 50; ++i) {
        visualizer_.update(dummyFreq, 0.016f);
      }
    });
  }
}

// Test 7: Audio data variation
TEST_F(RealPresetTest, AudioDataVariation) {
  std::string simplePreset = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time);

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index);
)";

  auto parseResult = parser_.parsePreset(simplePreset, "audio_test.milk");
  EXPECT_TRUE(parseResult.has_value());

  if (parseResult.has_value()) {
    visualizer_.loadPreset(simplePreset, "audio_test.milk");

    // Test with different frequency distributions
    std::vector<std::vector<float>> audioVariations = {
      std::vector<float>(256, 0.0f),   // Silent
      std::vector<float>(256, 1.0f),   // Full volume
      std::vector<float>(256, 0.5f),   // Medium
    };

    for (const auto& freqBins : audioVariations) {
      EXPECT_NO_THROW({
        visualizer_.update(freqBins, 0.016f);
      }) << "Failed with audio variation";
    }
  }
}

// Test 8: Preset state reset
TEST_F(RealPresetTest, PresetStateReset) {
  std::string simplePreset = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time);

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index);
)";

  auto parseResult = parser_.parsePreset(simplePreset, "reset_test.milk");
  EXPECT_TRUE(parseResult.has_value());

  std::vector<float> dummyFreq(256, 0.5f);

  // Load, run frames, reset, run again
  visualizer_.loadPreset(simplePreset, "reset_test.milk");

  for (int i = 0; i < 50; ++i) {
    visualizer_.update(dummyFreq, 0.016f);
  }

  // Reset and run again
  EXPECT_NO_THROW({
    visualizer_.reset();
    for (int i = 0; i < 50; ++i) {
      visualizer_.update(dummyFreq, 0.016f);
    }
  });
}

}  // namespace milkdrop
