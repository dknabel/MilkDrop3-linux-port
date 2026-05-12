// src/platform/types.h
#pragma once
#include <string>
#include <vector>
#include <array>
#include <cstdint>

struct InputState {
  bool key_escape;
  bool key_p;           // pause/play
  bool key_f;           // fullscreen toggle
  bool key_s;           // show preset list
  bool key_left;        // previous preset
  bool key_right;       // next preset
  bool key_up;          // parameter up
  bool key_down;        // parameter down
  bool key_q;           // quit
};

struct AudioFrame {
  std::vector<float> samples;
  int sampleRate;
  int channels;
};

enum class BlendMode {
  Replace,
  Add,
  Multiply,
  Alpha
};

enum class RenderCommandType {
  Clear,
  DrawWaveform,
  DrawShape,
  DrawCustom
};

struct RenderCommand {
  RenderCommandType type;

  // Clear command data
  std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

  // Waveform command data
  std::vector<float> frequencyBins;
  std::array<float, 4> waveColor = {1.0f, 1.0f, 1.0f, 1.0f};

  // Shape command data
  std::array<float, 2> shapePosition = {0.5f, 0.5f};
  float shapeRadius = 0.1f;
  std::array<float, 4> shapeColor = {1.0f, 0.0f, 0.0f, 1.0f};

  // Legacy render command data (for backward compatibility)
  uint32_t shaderHandle = 0;
  uint32_t vertexBufferHandle = 0;
  uint32_t indexCount = 0;
  BlendMode blendMode = BlendMode::Alpha;
};

struct Config {
  std::string lastPreset;
  std::string audioDevice;
  int resolutionWidth = 1920;
  int resolutionHeight = 1080;
  bool fullscreen = false;
  float audioSensitivity = 1.0f;
};
