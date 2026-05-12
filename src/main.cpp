// src/main.cpp
#include <iostream>
#include <memory>

#include "platform/audio.h"
#include "platform/window.h"
#include "platform/graphics.h"
#include "platform/config.h"
#include "audio/audio_analyzer.h"
#include "ui/preset_manager.h"

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

    isRunning_ = true;
    return true;
  }

  void shutdown() {
    std::cout << "Shutting down Milkdrop3...\n";
    isRunning_ = false;

    graphics_.reset();
    window_.reset();
    audio_.reset();

    auto& configMgr = ConfigManager::getInstance();
    configMgr.save();
  }

  void run() {
    if (!initialize()) {
      std::cerr << "Failed to initialize application\n";
      return;
    }

    std::cout << "Starting main loop...\n";

    while (isRunning_ && !window_->shouldClose()) {
      update();
      render();
    }

    shutdown();
    std::cout << "Application terminated normally\n";
  }

private:
  std::unique_ptr<Window> window_;
  std::unique_ptr<GraphicsDevice> graphics_;
  std::unique_ptr<AudioInput> audio_;
  std::unique_ptr<PresetManager> presetMgr_;
  std::unique_ptr<AudioAnalyzer> audioAnalyzer_;

  std::string currentPreset_;
  bool isRunning_ = false;

  void update() {
    window_->update();

    // Handle input
    InputState input;
    window_->getInputState(input);

    if (input.key_escape || input.key_q) {
      isRunning_ = false;
    }

    if (input.key_left) {
      presetMgr_->previousPreset();
      currentPreset_ = presetMgr_->loadPresetByIndex(presetMgr_->getCurrentPresetIndex());
    }

    if (input.key_right) {
      presetMgr_->nextPreset();
      currentPreset_ = presetMgr_->loadPresetByIndex(presetMgr_->getCurrentPresetIndex());
    }

    if (input.key_f) {
      // TODO: Toggle fullscreen
    }

    if (input.key_p) {
      // TODO: Pause/play
    }

    // Get audio frame
    AudioFrame audioFrame;
    if (audio_->getAudioFrame(audioFrame)) {
      auto freqBins = audioAnalyzer_->analyze(audioFrame.samples);
      // TODO: Pass frequency bins to visualization engine
    }
  }

  void render() {
    // Clear screen
    graphics_->clear(0.0f, 0.0f, 0.0f, 1.0f);

    // TODO: Execute visualization engine render commands
    // graphics_->executeRenderCommand(...);

    // Present
    graphics_->present();
  }
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
