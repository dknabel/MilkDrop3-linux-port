#include <gtest/gtest.h>
#include "core/preset/preset_parser.h"
#include "core/preset/preset_validator.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace milkdrop {

class PresetLibraryTest : public ::testing::Test {
protected:
  PresetParser parser;

  std::vector<std::string> findPresetFiles(const std::string& dir) {
    std::vector<std::string> presets;
    if (!fs::exists(dir)) return presets;

    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
      if (entry.path().extension() == ".milk") {
        presets.push_back(entry.path().string());
      }
    }
    return presets;
  }

  std::string readPresetFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    return std::string((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  }
};

// Test simple preset parsing
TEST_F(PresetLibraryTest, LoadSimplePreset) {
  std::string presetContent = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time);

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index);
)";

  auto result = parser.parsePreset(presetContent, "simple.milk");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result->filename, "simple.milk");
  EXPECT_FALSE(result->state.per_frame_eqs_code.empty());
}

// Test parsing multiple presets
TEST_F(PresetLibraryTest, ParseMultiplePresets) {
  // Create 3 simple presets and verify all parse successfully
  std::vector<std::string> presets = {
    "[per_frame_eqs_0]\nzoom = 1.0;",
    "[per_frame_eqs_0]\nrotation = time * 0.5;",
    "[wave_0_init]\ny = 0.0;\n[wave_0_per_point]\ny = sample(index);"
  };

  for (size_t i = 0; i < presets.size(); ++i) {
    auto result = parser.parsePreset(presets[i], "preset_" + std::to_string(i) + ".milk");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->filename, "preset_" + std::to_string(i) + ".milk");
  }
}

// Test validation of parsed presets
TEST_F(PresetLibraryTest, ValidateParsedPreset) {
  std::string presetContent = R"(
[per_frame_eqs_0]
zoom = 1.0;

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index);
)";

  auto parsed = parser.parsePreset(presetContent, "validation_test.milk");
  ASSERT_TRUE(parsed.has_value());

  auto validation = PresetValidator::validate(parsed.value());
  EXPECT_TRUE(validation.valid);
}

// Test sequential preset parsing
TEST_F(PresetLibraryTest, SequentialPresetLoading) {
  std::vector<std::string> presetContents = {
    "[per_frame_eqs_0]\nzoom = 1.0;",
    "[per_frame_eqs_0]\nzoom = 1.5;",
    "[per_frame_eqs_0]\nzoom = 2.0;"
  };

  for (size_t i = 0; i < presetContents.size(); ++i) {
    auto result = parser.parsePreset(presetContents[i], "seq_" + std::to_string(i) + ".milk");
    EXPECT_TRUE(result.has_value());

    // Verify we can validate each parsed preset
    auto validation = PresetValidator::validate(result.value());
    EXPECT_TRUE(validation.valid);
  }
}

// Test parsing preset with shapes and waves
TEST_F(PresetLibraryTest, PresetWithShapesAndWaves) {
  std::string presetContent = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.05 * sin(time);

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

  auto result = parser.parsePreset(presetContent, "complex.milk");
  EXPECT_TRUE(result.has_value());

  // Validate shapes were parsed
  EXPECT_TRUE(result->shapes[0].enabled);
  EXPECT_FALSE(result->shapes[0].init_code.empty());

  // Validate waves were parsed
  EXPECT_TRUE(result->waves[0].enabled);
  EXPECT_FALSE(result->waves[0].init_code.empty());

  // Validate the entire preset
  auto validation = PresetValidator::validate(result.value());
  EXPECT_TRUE(validation.valid);
}

// Test parsing preset with per-frame equations
TEST_F(PresetLibraryTest, PresetWithComplexEquations) {
  std::string presetContent = R"(
[per_frame_eqs_0]
zoom = 1.0 + 0.1 * sin(time);
rotation = time * 0.5;
cx = 0.5 + 0.1 * cos(time);
cy = 0.5 + 0.1 * sin(time);

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index);
)";

  auto result = parser.parsePreset(presetContent, "equations.milk");
  EXPECT_TRUE(result.has_value());

  // Verify per-frame equations were captured
  const auto& code = result->state.per_frame_eqs_code;
  EXPECT_NE(code.find("zoom"), std::string::npos);
  EXPECT_NE(code.find("rotation"), std::string::npos);
  EXPECT_NE(code.find("cx"), std::string::npos);
  EXPECT_NE(code.find("cy"), std::string::npos);
}

// Test parsing multiple shapes and waves
TEST_F(PresetLibraryTest, PresetWithMultipleShapesAndWaves) {
  std::string presetContent = R"(
[per_frame_eqs_0]
zoom = 1.0;

[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index);

[wave_1_init]
y = 0.5;

[wave_1_per_point]
y = sample(index + 0.1);

[shape_0_init]
x = 0.3;
y = 0.3;

[shape_0_per_frame]
x = 0.3 + 0.1 * sin(time);

[shape_1_init]
x = 0.7;
y = 0.7;

[shape_1_per_frame]
x = 0.7 + 0.1 * cos(time);
)";

  auto result = parser.parsePreset(presetContent, "multi_shapes_waves.milk");
  EXPECT_TRUE(result.has_value());

  // Verify multiple waves were parsed
  EXPECT_TRUE(result->waves[0].enabled);
  EXPECT_TRUE(result->waves[1].enabled);
  EXPECT_FALSE(result->waves[2].enabled);

  // Verify multiple shapes were parsed
  EXPECT_TRUE(result->shapes[0].enabled);
  EXPECT_TRUE(result->shapes[1].enabled);
  EXPECT_FALSE(result->shapes[2].enabled);

  // Validate the entire preset
  auto validation = PresetValidator::validate(result.value());
  EXPECT_TRUE(validation.valid);
}

// Test parsing preset library discovery
TEST_F(PresetLibraryTest, PresetLibraryDiscovery) {
  // Test that we can find preset files in a directory
  // Create a temporary directory with test presets
  fs::path tmpDir = fs::temp_directory_path() / "milkdrop_test_presets";
  fs::create_directories(tmpDir);

  // Create some test preset files
  for (int i = 0; i < 3; ++i) {
    fs::path presetPath = tmpDir / ("test_preset_" + std::to_string(i) + ".milk");
    std::ofstream file(presetPath);
    file << "[per_frame_eqs_0]\nzoom = 1.0;";
    file.close();
  }

  // Find the presets
  auto foundPresets = findPresetFiles(tmpDir.string());
  EXPECT_GE(foundPresets.size(), 3);

  // Verify they can be parsed
  for (const auto& presetPath : foundPresets) {
    auto content = readPresetFile(presetPath);
    EXPECT_FALSE(content.empty());

    auto result = parser.parsePreset(content, fs::path(presetPath).filename().string());
    EXPECT_TRUE(result.has_value());
  }

  // Cleanup
  fs::remove_all(tmpDir);
}

}  // namespace milkdrop
