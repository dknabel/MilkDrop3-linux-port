#pragma once
#include "preset_types.h"
#include <vector>
#include <string>

namespace milkdrop {

struct ValidationResult {
  bool valid = true;
  std::vector<std::string> errors;
  std::vector<std::string> warnings;

  void addError(const std::string& msg) { errors.push_back(msg); valid = false; }
  void addWarning(const std::string& msg) { warnings.push_back(msg); }

  void print() const;
};

class PresetValidator {
public:
  // Validate preset structure and content
  static ValidationResult validate(const Preset& preset);

  // Get diagnostic info
  static std::string getDiagnostics(const Preset& preset);

private:
  static void validateState(const PresetState& state, ValidationResult& result);
  static void validateShapes(const std::array<Shape, 16>& shapes, ValidationResult& result);
  static void validateWaves(const std::array<Wave, 4>& waves, ValidationResult& result);
};

} // namespace milkdrop
