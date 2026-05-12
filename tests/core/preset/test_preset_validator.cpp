#include <gtest/gtest.h>
#include "core/preset/preset_validator.h"

TEST(PresetValidatorTest, ValidateNormalPreset) {
  milkdrop::Preset preset;
  preset.filename = "test.milk";
  preset.state.zoom = 1.0f;
  preset.waves.resize(1);
  preset.waves[0].enabled = 1;
  preset.waves[0].points = 64;

  auto result = milkdrop::PresetValidator::validate(preset);
  EXPECT_TRUE(result.valid);
  EXPECT_TRUE(result.errors.empty());
}

TEST(PresetValidatorTest, DetectNaNValues) {
  milkdrop::Preset preset;
  preset.state.zoom = NAN;

  auto result = milkdrop::PresetValidator::validate(preset);
  EXPECT_FALSE(result.valid);
  EXPECT_FALSE(result.errors.empty());
}

TEST(PresetValidatorTest, WarnOnNoShapesOrWaves) {
  milkdrop::Preset preset;
  // All shapes and waves disabled by default

  auto result = milkdrop::PresetValidator::validate(preset);
  EXPECT_GE(result.warnings.size(), 2); // At least warnings for no shapes and no waves
}

TEST(PresetValidatorTest, DetectInvalidZoom) {
  milkdrop::Preset preset;
  preset.state.zoom = -1.0f;

  auto result = milkdrop::PresetValidator::validate(preset);
  EXPECT_GE(result.warnings.size(), 1);
}

TEST(PresetValidatorTest, GetDiagnostics) {
  milkdrop::Preset preset;
  preset.filename = "diag_test.milk";
  preset.state.zoom = 1.5f;
  preset.waves.resize(1);
  preset.waves[0].enabled = 1;
  preset.waves[0].points = 128;

  std::string diag = milkdrop::PresetValidator::getDiagnostics(preset);
  EXPECT_NE(diag.find("diag_test.milk"), std::string::npos);
  EXPECT_NE(diag.find("1.5"), std::string::npos);
  EXPECT_NE(diag.find("1 waves"), std::string::npos);
}
