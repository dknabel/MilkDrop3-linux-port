#pragma once
#include "../platform/types.h"
#include "preset/preset_types.h"
#include "preset/preset_parser.h"
#include "expression/expression_evaluator.h"
#include <string>
#include <vector>
#include <memory>

class VisualizationEngine {
public:
  VisualizationEngine();
  ~VisualizationEngine();

  // Load and execute a preset
  bool loadPreset(const std::string& presetContent, const std::string& filename);

  // Update with audio data and elapsed time
  void update(const std::vector<float>& frequencyBins, float deltaTime);

  // Get pending render commands
  std::vector<RenderCommand> getRenderCommands();

  // Reset animation state
  void reset();

  // Get current preset (for debugging/introspection)
  const milkdrop::Preset* getCurrentPreset() const { return currentPreset_.get(); }

private:
  std::unique_ptr<milkdrop::Preset> currentPreset_;
  milkdrop::PresetParser parser_;
  milkdrop::ExpressionEvaluator evaluator_;

  std::vector<RenderCommand> pendingCommands_;
  float elapsedTime_;

  // Helper methods
  void compilePresetExpressions(milkdrop::Preset& preset);
  void generateRenderCommands(const std::vector<float>& frequencyBins, float deltaTime);
  void evaluatePerFrameEquations(const std::vector<float>& frequencyBins);
};
