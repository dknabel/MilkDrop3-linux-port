// src/platform/types.h
#pragma once
#include <string>
#include <vector>
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

struct RenderCommand {
  // Simplified render command for visualization
  // Full implementation will depend on Milkdrop3 core
  uint32_t shaderHandle;
  uint32_t vertexBufferHandle;
  uint32_t indexCount;
  BlendMode blendMode;
};

struct Config {
  std::string lastPreset;
  std::string audioDevice;
  int resolutionWidth = 1920;
  int resolutionHeight = 1080;
  bool fullscreen = false;
  float audioSensitivity = 1.0f;
};
