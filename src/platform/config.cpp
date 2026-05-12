// src/platform/config.cpp
#include "config.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

ConfigManager& ConfigManager::getInstance() {
  static ConfigManager instance;
  return instance;
}

std::string ConfigManager::getConfigDir() const {
  // XDG Base Directory specification
  const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
  fs::path configDir;

  if (xdgConfig && fs::path(xdgConfig).is_absolute()) {
    configDir = fs::path(xdgConfig) / "milkdrop";
  } else {
    const char* home = std::getenv("HOME");
    if (!home) {
      throw std::runtime_error("HOME environment variable not set");
    }
    configDir = fs::path(home) / ".config" / "milkdrop";
  }

  // Create directory if it doesn't exist
  fs::create_directories(configDir);
  return configDir.string();
}

bool ConfigManager::load() {
  try {
    configPath_ = getConfigDir() + "/config.json";

    std::ifstream file(configPath_);
    if (!file.is_open()) {
      // First run, use defaults
      return true;
    }

    // TODO: Parse JSON (for MVP, use defaults)
    // Full implementation will use nlohmann/json or similar
    return true;
  } catch (const std::exception& e) {
    // Log and use defaults
    return true;
  }
}

bool ConfigManager::save() {
  try {
    configPath_ = getConfigDir() + "/config.json";

    std::ofstream file(configPath_);
    if (!file.is_open()) {
      return false;
    }

    // TODO: Serialize config to JSON
    // For MVP, write minimal JSON manually
    file << "{\n";
    file << "  \"lastPreset\": \"" << config_.lastPreset << "\",\n";
    file << "  \"audioDevice\": \"" << config_.audioDevice << "\",\n";
    file << "  \"resolutionWidth\": " << config_.resolutionWidth << ",\n";
    file << "  \"resolutionHeight\": " << config_.resolutionHeight << ",\n";
    file << "  \"fullscreen\": " << (config_.fullscreen ? "true" : "false") << ",\n";
    file << "  \"audioSensitivity\": " << config_.audioSensitivity << "\n";
    file << "}\n";

    return true;
  } catch (const std::exception& e) {
    return false;
  }
}

Config& ConfigManager::getConfig() {
  return config_;
}

const Config& ConfigManager::getConfig() const {
  return config_;
}
