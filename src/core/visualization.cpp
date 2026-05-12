#include "visualization.h"
#include <iostream>
#include <cmath>

namespace milkdrop {
constexpr int FREQUENCY_BINS = 64;
}  // namespace milkdrop

VisualizationEngine::VisualizationEngine()
    : currentPreset_(nullptr), elapsedTime_(0.0f) {
  // Initialize expression evaluator
  if (!evaluator_.initialize()) {
    std::cerr << "Failed to initialize EEL2 evaluator\n";
  }
}

VisualizationEngine::~VisualizationEngine() {
  // Cleanup handled by unique_ptr and EEL2VM destructor
}

bool VisualizationEngine::loadPreset(const std::string& presetContent,
                                      const std::string& filename) {
  // Parse preset from content
  auto parsed = parser_.parsePreset(presetContent, filename);
  if (!parsed) {
    std::cerr << "Failed to parse preset: " << parser_.getLastError() << "\n";
    return false;
  }

  // Store as current preset
  currentPreset_ = std::make_unique<milkdrop::Preset>(parsed.value());

  // Register variables in evaluator
  evaluator_.registerVariable("time");
  evaluator_.registerVariable("fps");
  evaluator_.registerVariable("frame");

  // Register audio variables (per-frame analysis)
  for (int i = 0; i < milkdrop::FREQUENCY_BINS; ++i) {
    std::string varName = "q" + std::to_string(i);
    evaluator_.registerVariable(varName);
  }

  // Register preset state variables
  evaluator_.registerVariable("zoom");
  evaluator_.registerVariable("rotation");
  evaluator_.registerVariable("center_x");
  evaluator_.registerVariable("center_y");
  evaluator_.registerVariable("decay");
  evaluator_.registerVariable("brighten");
  evaluator_.registerVariable("wave_mode");
  evaluator_.registerVariable("wave_alpha");
  evaluator_.registerVariable("wave_scale");

  // Compile all preset expressions
  compilePresetExpressions(*currentPreset_);

  return true;
}

void VisualizationEngine::compilePresetExpressions(milkdrop::Preset& preset) {
  // Compile per-frame equations
  if (!preset.state.per_frame_eqs_code.empty()) {
    auto handle = evaluator_.compile(preset.state.per_frame_eqs_code);
    if (handle) {
      preset.state.per_frame_eqs_handle = handle;
    }
  }

  // Compile shape equations
  for (size_t i = 0; i < preset.shapes.size(); ++i) {
    if (!preset.shapes[i].enabled) continue;

    // Compile per-frame equations
    if (!preset.shapes[i].per_frame_code.empty()) {
      auto handle = evaluator_.compile(preset.shapes[i].per_frame_code);
      if (handle) {
        preset.shapes[i].per_frame_handle = handle;
      }
    }
  }

  // Compile wave equations
  for (size_t i = 0; i < preset.waves.size(); ++i) {
    if (!preset.waves[i].enabled) continue;

    // Compile per-point equations
    if (!preset.waves[i].per_point_code.empty()) {
      auto handle = evaluator_.compile(preset.waves[i].per_point_code);
      if (handle) {
        preset.waves[i].per_point_handle = handle;
      }
    }
  }
}

void VisualizationEngine::update(const std::vector<float>& frequencyBins,
                                   float deltaTime) {
  if (!currentPreset_) {
    // No preset loaded, just return empty commands
    pendingCommands_.clear();
    return;
  }

  elapsedTime_ += deltaTime;

  // Update time variables
  double* timeVar = evaluator_.getVariable("time");
  if (timeVar) *timeVar = elapsedTime_;

  double* frameVar = evaluator_.getVariable("frame");
  if (frameVar) *frameVar = static_cast<float>(static_cast<int>(elapsedTime_ * 60.0f));

  // Evaluate per-frame equations
  evaluatePerFrameEquations(frequencyBins);

  // Generate render commands
  generateRenderCommands(frequencyBins, deltaTime);
}

void VisualizationEngine::evaluatePerFrameEquations(
    const std::vector<float>& frequencyBins) {
  if (!currentPreset_) return;

  milkdrop::PresetState& state = currentPreset_->state;

  // Update audio bucket variables (q[0..63])
  int bucketCount = std::min(milkdrop::FREQUENCY_BINS, static_cast<int>(frequencyBins.size()));
  for (int i = 0; i < bucketCount; ++i) {
    std::string varName = "q" + std::to_string(i);
    double* var = evaluator_.getVariable(varName);
    if (var) {
      *var = frequencyBins[i];
    }
  }

  // Execute per-frame equations
  if (state.per_frame_eqs_handle) {
    evaluator_.execute(state.per_frame_eqs_handle);

    // Read back modified values
    double* val = nullptr;

    val = evaluator_.getVariable("zoom");
    if (val) state.zoom = *val;

    val = evaluator_.getVariable("rotation");
    if (val) state.rotation = *val;

    val = evaluator_.getVariable("center_x");
    if (val) state.center_x = *val;

    val = evaluator_.getVariable("center_y");
    if (val) state.center_y = *val;

    val = evaluator_.getVariable("decay");
    if (val) state.decay = *val;

    val = evaluator_.getVariable("brighten");
    if (val) state.brighten = *val;

    val = evaluator_.getVariable("wave_alpha");
    if (val) state.wave_alpha = *val;

    val = evaluator_.getVariable("wave_scale");
    if (val) state.wave_scale = *val;
  }

  // Execute shape per-frame equations
  for (size_t i = 0; i < currentPreset_->shapes.size(); ++i) {
    if (!currentPreset_->shapes[i].enabled) continue;

    milkdrop::Shape& shape = currentPreset_->shapes[i];

    // Execute per-frame
    if (shape.per_frame_handle) {
      evaluator_.execute(shape.per_frame_handle);

      // Shape-specific variables would be read back here if tracked
    }
  }

  // Execute wave per-point equations
  for (size_t i = 0; i < currentPreset_->waves.size(); ++i) {
    if (!currentPreset_->waves[i].enabled) continue;

    milkdrop::Wave& wave = currentPreset_->waves[i];

    // Execute per-point equations
    if (wave.per_point_handle) {
      evaluator_.execute(wave.per_point_handle);
    }
  }
}

void VisualizationEngine::generateRenderCommands(
    const std::vector<float>& frequencyBins, float deltaTime) {
  if (!currentPreset_) return;

  pendingCommands_.clear();

  const milkdrop::PresetState& state = currentPreset_->state;

  // TODO (Task 6): Implement full render command generation
  // - Generate clear command
  // - Generate waveform rendering commands (from waves)
  // - Generate shape rendering commands (from shapes)
  // - Generate custom warp/blend commands from preset state

  // For now: generate clear command only
  RenderCommand clearCmd;
  clearCmd.shaderHandle = 0;
  clearCmd.vertexBufferHandle = 0;
  clearCmd.indexCount = 0;
  clearCmd.blendMode = BlendMode::Replace;
  pendingCommands_.push_back(clearCmd);
}

std::vector<RenderCommand> VisualizationEngine::getRenderCommands() {
  // Return and clear pending commands
  std::vector<RenderCommand> commands = pendingCommands_;
  pendingCommands_.clear();
  return commands;
}

void VisualizationEngine::reset() {
  elapsedTime_ = 0.0f;
  pendingCommands_.clear();
  currentPreset_ = nullptr;
}
