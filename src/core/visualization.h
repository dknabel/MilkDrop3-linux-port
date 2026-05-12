#pragma once
#include "../platform/types.h"
#include <string>
#include <vector>
#include <memory>

class VisualizationEngine {
public:
  VisualizationEngine();
  ~VisualizationEngine();

  // Load and execute a preset
  bool loadPreset(const std::string& presetContent);

  // Update with audio data and elapsed time
  void update(const std::vector<float>& frequencyBins, float deltaTime);

  // Get pending render commands
  std::vector<RenderCommand> getRenderCommands();

  // Reset animation state
  void reset();

private:
  // Milkdrop3 core components will go here (Task 3+)
  // For now: just structure for future integration
  std::vector<RenderCommand> pendingCommands_;
};
