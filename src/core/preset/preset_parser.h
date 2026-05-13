#pragma once

#include "preset_types.h"
#include <string>
#include <memory>
#include <optional>
#include <map>

namespace milkdrop {

class PresetParser {
public:
  PresetParser();
  ~PresetParser();

  // Parse preset from file content (INI format)
  std::optional<Preset> parsePreset(const std::string& content,
                                     const std::string& filename);

  // Get last error message
  const std::string& getLastError() const { return lastError_; }

private:
  static constexpr int MAX_SHAPES = 16;
  static constexpr int MAX_WAVES = 4;

  std::string lastError_;

  // Parse section helpers for Milkdrop3 property-based format
  void parseWave(int waveIndex, const std::map<std::string, std::string>& props,
                 Preset& preset);
  void parseShape(int shapeIndex, const std::map<std::string, std::string>& props,
                  Preset& preset);
};

}  // namespace milkdrop
