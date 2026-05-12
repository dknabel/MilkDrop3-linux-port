#pragma once

#include "preset_types.h"
#include <string>
#include <memory>
#include <optional>

namespace milkdrop {

class PresetParser {
public:
  PresetParser();
  ~PresetParser();

  // Parse preset from file content (INI format)
  std::optional<Preset> parsePreset(const std::string& content,
                                     const std::string& filename);

  // Get last error message
  std::string getLastError() const { return lastError_; }

private:
  std::string lastError_;

  // Helper to extract and filter section content
  std::string extractSection(const std::string& sectionName,
                             const std::string& content);

  // Parse section helpers
  void parseSection(const std::string& section, const std::string& content,
                    Preset& preset);
  void parsePerFrameEquations(const std::string& content, PresetState& state);
  void parseWarpShaderCode(const std::string& content, Preset& preset);
  void parseShape(int shapeIndex, const std::string& content, Preset& preset);
  void parseWave(int waveIndex, const std::string& content, Preset& preset);

  // INI parsing utility
  std::optional<std::string> getValueForKey(const std::string& section,
                                             const std::string& key,
                                             const std::string& content);
};

}  // namespace milkdrop
