#include "visualization.h"
#include <iostream>

VisualizationEngine::VisualizationEngine() {
  // Initialize default state, empty render queue
  pendingCommands_.clear();
}

VisualizationEngine::~VisualizationEngine() {
  // Clean up (no-op for now)
}

bool VisualizationEngine::loadPreset(const std::string& presetContent) {
  // Parse preset DSL (placeholder - will be filled in Task 3)
  // For MVP: Accept string, validate format, print debug message
  if (presetContent.empty()) {
    std::cerr << "Warning: Empty preset content provided" << std::endl;
    return false;
  }

  std::cout << "Debug: Loaded preset of size " << presetContent.size() << " bytes" << std::endl;

  // Placeholder validation: just check if content looks reasonable
  // Task 3 will implement actual DSL parsing
  return true;
}

void VisualizationEngine::update(const std::vector<float>& frequencyBins,
                                   float deltaTime) {
  // Accept frequency bins and time, generate render commands (placeholder)
  // For MVP: Generate a simple test render command (clear screen)

  // Clear previous commands
  pendingCommands_.clear();

  // Generate a simple test render command
  RenderCommand testCommand;
  testCommand.shaderHandle = 0;  // Default/uninitialized
  testCommand.vertexBufferHandle = 0;
  testCommand.indexCount = 0;
  testCommand.blendMode = BlendMode::Replace;

  pendingCommands_.push_back(testCommand);

  // Debug output
  std::cout << "Debug: update() called with " << frequencyBins.size()
            << " frequency bins, deltaTime: " << deltaTime << " seconds"
            << std::endl;
}

std::vector<RenderCommand> VisualizationEngine::getRenderCommands() {
  // Return and clear pending commands
  std::vector<RenderCommand> commands = pendingCommands_;
  pendingCommands_.clear();
  return commands;
}

void VisualizationEngine::reset() {
  // Clear command queue and reset state
  pendingCommands_.clear();
  std::cout << "Debug: VisualizationEngine reset" << std::endl;
}
