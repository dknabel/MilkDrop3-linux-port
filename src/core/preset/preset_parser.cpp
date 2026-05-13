// src/core/preset/preset_parser.cpp
#include "preset_parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace milkdrop {

PresetParser::PresetParser() = default;

PresetParser::~PresetParser() = default;

// Helper to trim whitespace
static std::string trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, (last - first + 1));
}

// Helper to lowercase a string
static std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return str;
}

// Parse a float value from string
static float parseFloat(const std::string& value, float defaultVal = 0.0f) {
  try {
    return std::stof(value);
  } catch (...) {
    return defaultVal;
  }
}

// Parse an int value from string
static int parseInt(const std::string& value, int defaultVal = 0) {
  try {
    return std::stoi(value);
  } catch (...) {
    return defaultVal;
  }
}

std::optional<Preset> PresetParser::parsePreset(const std::string& content,
                                                 const std::string& filename) {
  if (content.empty()) {
    lastError_ = "Preset content is empty";
    return std::nullopt;
  }

  Preset preset;
  preset.filename = filename;

  // Initialize shapes and waves vectors
  preset.shapes.resize(MAX_SHAPES);
  preset.waves.resize(MAX_WAVES);

  // Parse global preset properties
  std::istringstream stream(content);
  std::string line;
  std::map<std::string, std::string> properties;

  // First pass: collect all key=value pairs
  while (std::getline(stream, line)) {
    line = trim(line);

    // Skip empty lines and comments
    if (line.empty() || line[0] == ';' || line[0] == '#') continue;

    // Skip section headers like [PRESET1_BEGIN]
    if (line[0] == '[') continue;

    // Parse key=value
    size_t eqPos = line.find('=');
    if (eqPos != std::string::npos) {
      std::string key = trim(line.substr(0, eqPos));
      std::string value = trim(line.substr(eqPos + 1));
      properties[toLower(key)] = value;
    }
  }

  // Parse global state properties
  if (properties.count("fgammaadj"))
    preset.state.gamma_adj = parseFloat(properties["fgammaadj"], 1.0f);
  if (properties.count("fdecay"))
    preset.state.decay = parseFloat(properties["fdecay"], 0.95f);
  if (properties.count("fvideoechozoom"))
    preset.state.video_echo_zoom = parseFloat(properties["fvideoechozoom"], 1.0f);
  if (properties.count("fvideoechoalpha"))
    preset.state.video_echo_alpha = parseFloat(properties["fvideoechoalpha"], 0.0f);
  if (properties.count("nwavemode"))
    preset.state.wave_mode = parseInt(properties["nwavemode"], 0);
  if (properties.count("badditivewaves"))
    preset.state.additive_waves = parseInt(properties["badditivewaves"], 0) != 0;
  if (properties.count("fwavealpha"))
    preset.state.wave_alpha = parseFloat(properties["fwavealpha"], 0.5f);
  if (properties.count("fwavescale"))
    preset.state.wave_scale = parseFloat(properties["fwavescale"], 1.0f);
  if (properties.count("zoom"))
    preset.state.zoom = parseFloat(properties["zoom"], 1.0f);
  if (properties.count("rot"))
    preset.state.rotation = parseFloat(properties["rot"], 0.0f);
  if (properties.count("cx"))
    preset.state.center_x = parseFloat(properties["cx"], 0.5f);
  if (properties.count("cy"))
    preset.state.center_y = parseFloat(properties["cy"], 0.5f);
  if (properties.count("sx"))
    preset.state.stretch_x = parseFloat(properties["sx"], 1.0f);
  if (properties.count("sy"))
    preset.state.stretch_y = parseFloat(properties["sy"], 1.0f);
  if (properties.count("wave_r"))
    preset.state.wave_r = parseFloat(properties["wave_r"], 1.0f);
  if (properties.count("wave_g"))
    preset.state.wave_g = parseFloat(properties["wave_g"], 1.0f);
  if (properties.count("wave_b"))
    preset.state.wave_b = parseFloat(properties["wave_b"], 1.0f);

  // Parse per-frame equations
  for (int i = 0; i < 32; ++i) {
    std::string key = "per_frame_init_" + std::to_string(i);
    if (properties.count(key)) {
      if (i == 0) preset.state.per_frame_init_code = properties[key];
    }
  }

  for (int i = 1; i < 32; ++i) {
    std::string key = "per_frame_" + std::to_string(i);
    if (properties.count(key)) {
      if (!preset.state.per_frame_eqs_code.empty())
        preset.state.per_frame_eqs_code += "\n";
      preset.state.per_frame_eqs_code += properties[key];
    }
  }

  // Parse waves
  for (int i = 0; i < MAX_WAVES; ++i) {
    parseWave(i, properties, preset);
  }

  // Parse shapes
  for (int i = 0; i < MAX_SHAPES; ++i) {
    parseShape(i, properties, preset);
  }

  return preset;
}

void PresetParser::parseWave(int waveIndex,
                             const std::map<std::string, std::string>& props,
                             Preset& preset) {
  Wave& wave = preset.waves[waveIndex];

  std::string prefix = "wavecode_" + std::to_string(waveIndex) + "_";
  std::string enableKey = prefix + "enabled";

  // Check if wave is enabled
  if (props.count(enableKey)) {
    wave.enabled = parseInt(props.at(enableKey), 0);
  } else {
    wave.enabled = 0;
    return;  // Skip parsing if not enabled
  }

  // Parse wave properties
  if (props.count(prefix + "samples"))
    wave.points = parseInt(props.at(prefix + "samples"), 512);
  if (props.count(prefix + "busedots"))
    wave.use_dots = parseInt(props.at(prefix + "busedots"), 0);
  if (props.count(prefix + "bdrawthick"))
    wave.thick = parseInt(props.at(prefix + "bdrawthick"), 0);
  if (props.count(prefix + "badditive"))
    wave.additive = parseInt(props.at(prefix + "badditive"), 0);

  // Parse colors
  if (props.count(prefix + "r"))
    wave.r = parseFloat(props.at(prefix + "r"), 1.0f);
  if (props.count(prefix + "g"))
    wave.g = parseFloat(props.at(prefix + "g"), 1.0f);
  if (props.count(prefix + "b"))
    wave.b = parseFloat(props.at(prefix + "b"), 1.0f);
  if (props.count(prefix + "a"))
    wave.a = parseFloat(props.at(prefix + "a"), 1.0f);

  // Parse per-frame equations for this wave
  std::string perFrameKey = "wave_" + std::to_string(waveIndex) + "_per_frame";
  for (int j = 1; j < 16; ++j) {
    std::string key = perFrameKey + std::to_string(j);
    if (props.count(key)) {
      if (!wave.per_frame_code.empty())
        wave.per_frame_code += "\n";
      wave.per_frame_code += props.at(key);
    }
  }

  // Parse per-point equations for this wave
  std::string perPointKey = "wave_" + std::to_string(waveIndex) + "_per_point";
  for (int j = 1; j < 16; ++j) {
    std::string key = perPointKey + std::to_string(j);
    if (props.count(key)) {
      if (!wave.per_point_code.empty())
        wave.per_point_code += "\n";
      wave.per_point_code += props.at(key);
    }
  }

}

void PresetParser::parseShape(int shapeIndex,
                              const std::map<std::string, std::string>& props,
                              Preset& preset) {
  Shape& shape = preset.shapes[shapeIndex];

  std::string prefix = "shapecode_" + std::to_string(shapeIndex) + "_";
  std::string enableKey = prefix + "enabled";

  // Check if shape is enabled
  if (props.count(enableKey)) {
    shape.enabled = parseInt(props.at(enableKey), 0);
  } else {
    shape.enabled = 0;
    return;  // Skip parsing if not enabled
  }

  // Parse shape properties
  if (props.count(prefix + "sides"))
    shape.sides = parseInt(props.at(prefix + "sides"), 4);
  if (props.count(prefix + "additive"))
    shape.additive = parseInt(props.at(prefix + "additive"), 0);
  if (props.count(prefix + "thickoutline"))
    shape.additive = parseInt(props.at(prefix + "thickoutline"), 0);
  if (props.count(prefix + "textured"))
    shape.textured = parseInt(props.at(prefix + "textured"), 0);

  // Parse position and size
  if (props.count(prefix + "x"))
    shape.x = parseFloat(props.at(prefix + "x"), 0.5f);
  if (props.count(prefix + "y"))
    shape.y = parseFloat(props.at(prefix + "y"), 0.5f);
  if (props.count(prefix + "rad"))
    shape.radius = parseFloat(props.at(prefix + "rad"), 0.1f);
  if (props.count(prefix + "ang"))
    shape.angle = parseFloat(props.at(prefix + "ang"), 0.0f);

  // Parse texture properties
  if (props.count(prefix + "tex_ang"))
    shape.tex_angle = parseFloat(props.at(prefix + "tex_ang"), 0.0f);
  if (props.count(prefix + "tex_zoom"))
    shape.tex_zoom = parseFloat(props.at(prefix + "tex_zoom"), 1.0f);

  // Parse primary color
  if (props.count(prefix + "r"))
    shape.r = parseFloat(props.at(prefix + "r"), 1.0f);
  if (props.count(prefix + "g"))
    shape.g = parseFloat(props.at(prefix + "g"), 1.0f);
  if (props.count(prefix + "b"))
    shape.b = parseFloat(props.at(prefix + "b"), 1.0f);
  if (props.count(prefix + "a"))
    shape.a = parseFloat(props.at(prefix + "a"), 1.0f);

  // Parse secondary color
  if (props.count(prefix + "r2"))
    shape.r2 = parseFloat(props.at(prefix + "r2"), 1.0f);
  if (props.count(prefix + "g2"))
    shape.g2 = parseFloat(props.at(prefix + "g2"), 1.0f);
  if (props.count(prefix + "b2"))
    shape.b2 = parseFloat(props.at(prefix + "b2"), 1.0f);
  if (props.count(prefix + "a2"))
    shape.a2 = parseFloat(props.at(prefix + "a2"), 1.0f);

  // Parse border color
  if (props.count(prefix + "border_r"))
    shape.border_r = parseFloat(props.at(prefix + "border_r"), 0.0f);
  if (props.count(prefix + "border_g"))
    shape.border_g = parseFloat(props.at(prefix + "border_g"), 0.0f);
  if (props.count(prefix + "border_b"))
    shape.border_b = parseFloat(props.at(prefix + "border_b"), 0.0f);
  if (props.count(prefix + "border_a"))
    shape.border_a = parseFloat(props.at(prefix + "border_a"), 1.0f);

  // Parse init equations
  std::string initKey = "shape_" + std::to_string(shapeIndex) + "_init";
  for (int j = 1; j < 16; ++j) {
    std::string key = initKey + std::to_string(j);
    if (props.count(key)) {
      if (!shape.init_code.empty())
        shape.init_code += "\n";
      shape.init_code += props.at(key);
    }
  }

  // Parse per-frame equations
  std::string perFrameKey = "shape_" + std::to_string(shapeIndex) + "_per_frame";
  for (int j = 1; j < 32; ++j) {
    std::string key = perFrameKey + std::to_string(j);
    if (props.count(key)) {
      if (!shape.per_frame_code.empty())
        shape.per_frame_code += "\n";
      shape.per_frame_code += props.at(key);
    }
  }

}

}  // namespace milkdrop
