#include "preset_validator.h"
#include <iostream>
#include <sstream>
#include <cmath>

namespace milkdrop {

void ValidationResult::print() const {
  if (valid) {
    std::cout << "✓ Preset valid\n";
  } else {
    std::cout << "✗ Preset invalid\n";
  }

  for (const auto& err : errors) {
    std::cout << "  ERROR: " << err << "\n";
  }

  for (const auto& warn : warnings) {
    std::cout << "  WARNING: " << warn << "\n";
  }
}

ValidationResult PresetValidator::validate(const Preset& preset) {
  ValidationResult result;

  // Check filename
  if (preset.filename.empty()) {
    result.addWarning("No filename provided");
  }

  // Validate state
  validateState(preset.state, result);

  // Validate shapes
  std::array<Shape, 16> shapes_array = {};
  for (size_t i = 0; i < std::min(preset.shapes.size(), size_t(16)); ++i) {
    shapes_array[i] = preset.shapes[i];
  }
  validateShapes(shapes_array, result);

  // Validate waves
  std::array<Wave, 4> waves_array = {};
  for (size_t i = 0; i < std::min(preset.waves.size(), size_t(4)); ++i) {
    waves_array[i] = preset.waves[i];
  }
  validateWaves(waves_array, result);

  return result;
}

void PresetValidator::validateState(const PresetState& state, ValidationResult& result) {
  // Check for NaN/Inf in critical fields
  if (std::isnan(state.zoom) || std::isinf(state.zoom)) {
    result.addError("zoom contains NaN or Inf");
  }

  if (state.zoom <= 0.0f) {
    result.addWarning("zoom is <= 0 (should be positive)");
  }

  if (std::isnan(state.rotation) || std::isinf(state.rotation)) {
    result.addError("rotation contains NaN or Inf");
  }

  // Validate center coordinates are in reasonable range
  if (state.center_x < 0.0f || state.center_x > 1.0f) {
    result.addWarning("center_x outside [0, 1] range");
  }

  if (state.center_y < 0.0f || state.center_y > 1.0f) {
    result.addWarning("center_y outside [0, 1] range");
  }

  // Validate color values
  if (state.wave_r < 0.0f || state.wave_r > 1.0f ||
      state.wave_g < 0.0f || state.wave_g > 1.0f ||
      state.wave_b < 0.0f || state.wave_b > 1.0f) {
    result.addWarning("wave color values outside [0, 1] range");
  }
}

void PresetValidator::validateShapes(const std::array<Shape, 16>& shapes, ValidationResult& result) {
  int enabledCount = 0;

  for (int i = 0; i < 16; ++i) {
    if (!shapes[i].enabled) continue;

    enabledCount++;

    if (shapes[i].radius <= 0.0f) {
      result.addWarning("shape " + std::to_string(i) + " radius <= 0");
    }

    if (shapes[i].sides < 3) {
      result.addWarning("shape " + std::to_string(i) + " sides < 3");
    }
  }

  if (enabledCount == 0) {
    result.addWarning("No shapes enabled");
  }
}

void PresetValidator::validateWaves(const std::array<Wave, 4>& waves, ValidationResult& result) {
  int enabledCount = 0;

  for (int i = 0; i < 4; ++i) {
    if (!waves[i].enabled) continue;

    enabledCount++;

    if (waves[i].points <= 0) {
      result.addWarning("wave " + std::to_string(i) + " points <= 0");
    }
  }

  if (enabledCount == 0) {
    result.addWarning("No waves enabled");
  }
}

std::string PresetValidator::getDiagnostics(const Preset& preset) {
  std::ostringstream oss;

  oss << "Preset: " << preset.filename << "\n";
  oss << "State:\n";
  oss << "  zoom=" << preset.state.zoom << " rotation=" << preset.state.rotation << "\n";
  oss << "  center=(" << preset.state.center_x << ", " << preset.state.center_y << ")\n";

  int shapeCount = 0, waveCount = 0;
  for (const auto& s : preset.shapes) if (s.enabled) shapeCount++;
  for (const auto& w : preset.waves) if (w.enabled) waveCount++;

  oss << "  " << shapeCount << " shapes, " << waveCount << " waves\n";

  return oss.str();
}

} // namespace milkdrop
