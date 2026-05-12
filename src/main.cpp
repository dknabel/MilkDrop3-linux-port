// src/main.cpp
#include <iostream>
#include <memory>
#include <GLFW/glfw3.h>

#include "platform/audio.h"
#include "platform/window.h"
#include "platform/graphics.h"
#include "platform/config.h"
#include "audio/audio_analyzer.h"
#include "ui/preset_manager.h"
#include "ui/display_manager.h"
#include "core/visualization.h"

class Milkdrop3Application {
public:
  bool initialize() {
    std::cout << "Initializing Milkdrop3...\n";

    // Load configuration
    auto& configMgr = ConfigManager::getInstance();
    if (!configMgr.load()) {
      std::cerr << "Warning: failed to load config, using defaults\n";
    }

    // Create window
    window_ = std::unique_ptr<Window>(createWindow());
    if (!window_) {
      std::cerr << "Failed to create window\n";
      return false;
    }

    auto& config = configMgr.getConfig();
    if (!window_->create(config.resolutionWidth, config.resolutionHeight, config.fullscreen)) {
      std::cerr << "Failed to initialize window\n";
      return false;
    }

    // Create graphics device
    graphics_ = std::unique_ptr<GraphicsDevice>(createGraphicsDevice());
    if (!graphics_) {
      std::cerr << "Failed to create graphics device\n";
      return false;
    }

    if (!graphics_->initialize(window_.get())) {
      std::cerr << "Failed to initialize graphics device\n";
      return false;
    }

    // Create audio input
    audio_ = std::unique_ptr<AudioInput>(createAudioInput());
    if (!audio_) {
      std::cerr << "Failed to create audio input\n";
      return false;
    }

    if (!audio_->initialize(config.audioDevice)) {
      std::cerr << "Failed to initialize audio input\n";
      return false;
    }

    // Initialize preset manager
    presetMgr_ = std::make_unique<PresetManager>();
    if (!presetMgr_->scanPresets()) {
      std::cerr << "Warning: no presets found\n";
    }

    // Load first preset or default
    if (!presetMgr_->getPresets().empty()) {
      currentPreset_ = presetMgr_->loadPresetByIndex(0);
      std::cout << "Loaded initial preset\n";
    }

    audioAnalyzer_ = std::make_unique<AudioAnalyzer>(512);

    // Initialize display manager
    display_ = std::make_unique<DisplayManager>();
    if (!display_->initialize(1280, 720, "Milkdrop3 - Linux")) {
      std::cerr << "Failed to initialize display manager\n";
      return false;
    }

    // Initialize visualization engine
    visualizer_ = std::make_unique<VisualizationEngine>();
    if (!currentPreset_.empty()) {
      if (!visualizer_->loadPreset(currentPreset_, "")) {
        std::cerr << "Warning: failed to load initial preset in visualization engine\n";
      }
    }

    isRunning_ = true;
    return true;
  }

  void shutdown() {
    std::cout << "Shutting down Milkdrop3...\n";
    isRunning_ = false;

    // Shutdown visualization and display
    visualizer_.reset();
    if (display_) {
      display_->shutdown();
      display_.reset();
    }

    graphics_.reset();
    window_.reset();
    audio_.reset();

    auto& configMgr = ConfigManager::getInstance();
    configMgr.save();
  }

  bool run() {
    if (!initialize()) {
      std::cerr << "Application initialization failed\n";
      return false;
    }

    std::cout << "Starting main loop...\n";

    float deltaTime = 0.0f;
    double lastTime = display_->getElapsedTime();

    while (display_->isRunning()) {
      double currentTime = display_->getElapsedTime();
      deltaTime = static_cast<float>(currentTime - lastTime);
      lastTime = currentTime;

      // Handle keyboard input
      if (display_->isKeyPressed(GLFW_KEY_P)) {
        // Play/pause functionality (implement toggle)
        std::cout << "Play/pause toggled\n";
      }
      if (display_->isKeyPressed(GLFW_KEY_N)) {
        // Next preset
        if (presetMgr_->getPresets().size() > 0) {
          presetMgr_->nextPreset();
          currentPreset_ = presetMgr_->loadPresetByIndex(presetMgr_->getCurrentPresetIndex());
          visualizer_->loadPreset(currentPreset_, "");
          std::cout << "Loaded next preset\n";
        }
      }
      if (display_->isKeyPressed(GLFW_KEY_B)) {
        // Previous preset
        if (presetMgr_->getPresets().size() > 0) {
          presetMgr_->previousPreset();
          currentPreset_ = presetMgr_->loadPresetByIndex(presetMgr_->getCurrentPresetIndex());
          visualizer_->loadPreset(currentPreset_, "");
          std::cout << "Loaded previous preset\n";
        }
      }
      if (display_->isKeyPressed(GLFW_KEY_Q)) {
        // Quit
        break;
      }

      // Update visualization with audio
      AudioFrame audioFrame;
      if (audio_->getAudioFrame(audioFrame)) {
        if (!audioFrame.samples.empty()) {
          auto freqBins = audioAnalyzer_->analyze(audioFrame.samples);
          visualizer_->update(freqBins, deltaTime);
        }
      }

      // Render
      auto commands = visualizer_->getRenderCommands();
      display_->render(commands);

      // Update display (process events)
      if (!display_->update()) {
        break;
      }
    }

    shutdown();
    std::cout << "Application terminated normally\n";
    return true;
  }

private:
  std::unique_ptr<Window> window_;
  std::unique_ptr<GraphicsDevice> graphics_;
  std::unique_ptr<AudioInput> audio_;
  std::unique_ptr<PresetManager> presetMgr_;
  std::unique_ptr<AudioAnalyzer> audioAnalyzer_;
  std::unique_ptr<DisplayManager> display_;
  std::unique_ptr<VisualizationEngine> visualizer_;

  std::string currentPreset_;
  bool isRunning_ = false;

};

int main() {
  try {
    Milkdrop3Application app;
    app.run();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return 1;
  }
}
