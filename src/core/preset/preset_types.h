#pragma once

#include <string>
#include <vector>
#include <array>
#include <cmath>

// Forward declare EEL2 types
typedef void *NSEEL_VMCTX;
typedef void *NSEEL_CODEHANDLE;

namespace milkdrop {

// ============================================================================
// Preset Global Parameters
// ============================================================================

struct PresetState {
  // Post-processing parameters
  float gamma_adj = 1.0f;
  float video_echo_zoom = 0.9f;
  float video_echo_alpha = 0.0f;

  // Decay/blending
  float decay = 0.95f;

  // View transformation
  float zoom = 1.0f;
  float rotation = 0.0f;
  float warp_amount = 0.0f;
  float stretch_x = 1.0f;
  float stretch_y = 1.0f;

  // Center point
  float center_x = 0.5f;
  float center_y = 0.5f;

  // Brightness
  float brighten = 1.0f;

  // Waveform parameters
  int wave_mode = 0;
  float wave_alpha = 0.5f;
  float wave_scale = 1.0f;
  bool additive_waves = false;
  float wave_r = 1.0f;
  float wave_g = 1.0f;
  float wave_b = 1.0f;

  // Border parameters
  float outer_border_size = 0.0f;
  float outer_border_r = 0.0f;
  float outer_border_g = 0.0f;
  float outer_border_b = 0.0f;
  float outer_border_a = 0.0f;

  // Timing information (read-only from EEL2 perspective)
  float time = 0.0f;
  float fps = 60.0f;
  int frame = 0;

  // Audio analysis buckets (updated by visualization engine)
  std::array<float, 64> audio_buckets = {};  // q[0..63]

  // Temporary variables for expressions (t[0..7])
  std::array<float, 8> temp_vars = {};

  // Per-frame equations (as source code strings)
  std::string per_frame_init_code;
  std::string per_frame_eqs_code;

  // Per-pixel equations (for warping, evaluated later)
  std::string per_pixel_eqs_code;

  // Compiled code handles for EEL2
  NSEEL_CODEHANDLE per_frame_init_handle = nullptr;
  NSEEL_CODEHANDLE per_frame_eqs_handle = nullptr;

  // EEL2 VM context for evaluation
  NSEEL_VMCTX eel_vm = nullptr;

  // Preset metadata
  std::string name;
  std::string version = "2.0";
};

// ============================================================================
// Custom Shapes (MD3 supports up to 16)
// ============================================================================

struct Shape {
  int enabled = 0;
  int sides = 4;
  int additive = 0;
  int textured = 0;

  // Position and size
  float x = 0.5f;
  float y = 0.5f;
  float radius = 0.1f;
  float angle = 0.0f;

  // Color (RGBA)
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float a = 1.0f;

  // Secondary color
  float r2 = 1.0f;
  float g2 = 1.0f;
  float b2 = 1.0f;
  float a2 = 1.0f;

  // Border color
  float border_r = 0.0f;
  float border_g = 0.0f;
  float border_b = 0.0f;
  float border_a = 1.0f;
  float border_size = 0.0f;

  // Texture mapping
  float tex_angle = 0.0f;
  float tex_zoom = 1.0f;

  // EEL2 expressions
  std::string init_code;
  std::string per_frame_code;
  NSEEL_CODEHANDLE per_frame_handle = nullptr;
};

// ============================================================================
// Custom Waves (MD3 supports up to 4)
// ============================================================================

struct Wave {
  int enabled = 0;
  int mode = 0;        // 0=lines, 1=thick lines, 2=dots, 3=inverse, 4=invthick
  int use_dots = 0;
  int thick = 0;
  int additive = 0;
  int brighten = 0;

  // Color (RGBA)
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float a = 1.0f;

  // Position (for per-frame equations)
  float x = 0.5f;
  float y = 0.5f;

  // Number of points
  int points = 512;

  // EEL2 expressions
  std::string init_code;
  std::string per_frame_code;
  std::string per_point_code;
  NSEEL_CODEHANDLE per_frame_handle = nullptr;
  NSEEL_CODEHANDLE per_point_handle = nullptr;
};

// ============================================================================
// Complete Preset (all shapes, waves, state)
// ============================================================================

struct Preset {
  PresetState state;

  std::vector<Shape> shapes;   // Up to 16 in MD3
  std::vector<Wave> waves;     // Up to 4 in MD3

  // Shader code (for later - Task 6)
  std::string warp_shader_code;
  std::string composite_shader_code;

  // Metadata
  std::string filename;
  int preset_version = 202;
};

}  // namespace milkdrop
