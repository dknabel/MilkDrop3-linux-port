#include "preset_parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace milkdrop {

PresetParser::PresetParser() = default;

PresetParser::~PresetParser() = default;

// Helper function to extract section content and filter comments/empty lines
std::string PresetParser::extractSection(const std::string& sectionName,
                                         const std::string& content) {
  size_t pos = content.find(sectionName);
  if (pos == std::string::npos) {
    return "";
  }

  // Find the end of the section header line
  size_t sectionStartPos = content.find("\n", pos);
  if (sectionStartPos == std::string::npos) {
    return "";
  }
  sectionStartPos++;  // Move past the newline

  size_t endPos = content.find("\n[", sectionStartPos);
  if (endPos == std::string::npos)
    endPos = content.length();

  std::string section = content.substr(sectionStartPos, endPos - sectionStartPos);

  // Filter comments and empty lines
  std::istringstream iss(section);
  std::string line;
  std::ostringstream oss;

  while (std::getline(iss, line)) {
    if (line.empty())
      continue;

    // Trim leading whitespace
    size_t firstNonSpace = line.find_first_not_of(" \t\r");
    if (firstNonSpace == std::string::npos)
      continue;

    // Skip comment lines
    if (line[firstNonSpace] == ';')
      continue;

    oss << line << "\n";
  }

  return oss.str();
}

std::optional<Preset> PresetParser::parsePreset(const std::string& content,
                                                 const std::string& filename) {
  Preset preset;
  preset.filename = filename;

  // Initialize shapes vector (16 max in MD3)
  preset.shapes.resize(MAX_SHAPES);
  for (int i = 0; i < MAX_SHAPES; ++i) {
    preset.shapes[i].enabled = 0;
  }

  // Initialize waves vector (4 max in MD3)
  preset.waves.resize(MAX_WAVES);
  for (int i = 0; i < MAX_WAVES; ++i) {
    preset.waves[i].enabled = 0;
  }

  // Initialize default state
  preset.state.zoom = 1.0f;
  preset.state.rotation = 0.0f;
  preset.state.decay = 0.98f;
  preset.state.stretch_x = 1.0f;
  preset.state.stretch_y = 1.0f;
  preset.state.center_x = 0.5f;
  preset.state.center_y = 0.5f;
  preset.state.wave_alpha = 1.0f;
  preset.state.wave_scale = 1.0f;
  preset.state.wave_r = 1.0f;
  preset.state.wave_g = 1.0f;
  preset.state.wave_b = 1.0f;
  preset.state.gamma_adj = 1.0f;
  preset.state.video_echo_alpha = 0.0f;
  preset.state.video_echo_zoom = 1.0f;

  // Parse [per_frame_eqs_0] section
  parsePerFrameEquations(content, preset.state);

  // Parse [warp_shader_code] section
  parseWarpShaderCode(content, preset);

  // Parse shapes [shape_0_per_frame], [shape_0_init], etc.
  for (int i = 0; i < MAX_SHAPES; ++i) {
    parseShape(i, content, preset);
  }

  // Parse waves [wave_0_per_point], [wave_0_init], etc.
  for (int i = 0; i < MAX_WAVES; ++i) {
    parseWave(i, content, preset);
  }

  return preset;
}

void PresetParser::parsePerFrameEquations(const std::string& content,
                                          PresetState& state) {
  state.per_frame_eqs_code = extractSection("[per_frame_eqs_0]", content);
}

void PresetParser::parseWarpShaderCode(const std::string& content,
                                        Preset& preset) {
  preset.warp_shader_code = extractSection("[warp_shader_code]", content);
}

void PresetParser::parseShape(int shapeIndex, const std::string& content,
                               Preset& preset) {
  std::string initSection = "[shape_" + std::to_string(shapeIndex) + "_init]";
  std::string perFrameSection =
      "[shape_" + std::to_string(shapeIndex) + "_per_frame]";

  // Try to extract init section
  std::string initCode = extractSection(initSection, content);
  if (initCode.empty())
    return;  // Shape not defined

  // Mark as enabled
  preset.shapes[shapeIndex].enabled = 1;
  preset.shapes[shapeIndex].init_code = initCode;

  // Parse per_frame equations
  std::string pfCode = extractSection(perFrameSection, content);
  if (!pfCode.empty()) {
    preset.shapes[shapeIndex].per_frame_code = pfCode;
  }

  // Set defaults
  preset.shapes[shapeIndex].sides = 4;
  preset.shapes[shapeIndex].x = 0.5f;
  preset.shapes[shapeIndex].y = 0.5f;
  preset.shapes[shapeIndex].radius = 0.1f;
  preset.shapes[shapeIndex].angle = 0.0f;
  preset.shapes[shapeIndex].r = 1.0f;
  preset.shapes[shapeIndex].g = 1.0f;
  preset.shapes[shapeIndex].b = 1.0f;
  preset.shapes[shapeIndex].a = 1.0f;
}

void PresetParser::parseWave(int waveIndex, const std::string& content,
                              Preset& preset) {
  std::string initSection = "[wave_" + std::to_string(waveIndex) + "_init]";
  std::string perPointSection =
      "[wave_" + std::to_string(waveIndex) + "_per_point]";

  // Try to extract init section
  std::string initCode = extractSection(initSection, content);
  if (initCode.empty())
    return;  // Wave not defined

  // Mark as enabled
  preset.waves[waveIndex].enabled = 1;
  preset.waves[waveIndex].init_code = initCode;

  // Parse per_point equations
  std::string ppCode = extractSection(perPointSection, content);
  if (!ppCode.empty()) {
    preset.waves[waveIndex].per_point_code = ppCode;
  }

  // Set defaults
  preset.waves[waveIndex].r = 1.0f;
  preset.waves[waveIndex].g = 1.0f;
  preset.waves[waveIndex].b = 1.0f;
  preset.waves[waveIndex].a = 1.0f;
  preset.waves[waveIndex].mode = 0;  // Lines
  preset.waves[waveIndex].points = 64;
}

}  // namespace milkdrop
