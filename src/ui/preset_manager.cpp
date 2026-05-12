// src/ui/preset_manager.cpp
#include "preset_manager.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

PresetManager::PresetManager()
  : currentIndex_(0) {
}

std::vector<std::string> PresetManager::getPresetSearchPaths() const {
  std::vector<std::string> paths;

  // XDG user data directory
  const char* xdgData = std::getenv("XDG_DATA_HOME");
  if (xdgData && fs::path(xdgData).is_absolute()) {
    paths.push_back(std::string(xdgData) + "/milkdrop/presets");
  } else {
    const char* home = std::getenv("HOME");
    if (home) {
      paths.push_back(std::string(home) + "/.local/share/milkdrop/presets");
    }
  }

  // System-wide presets
  paths.push_back("/usr/share/milkdrop/presets");
  paths.push_back("/usr/local/share/milkdrop/presets");

  // Bundled presets relative to binary
  paths.push_back("./presets");
  paths.push_back("../share/presets");

  return paths;
}

void PresetManager::scanDirectory(const std::string& dir) {
  try {
    if (!fs::exists(dir)) {
      return;
    }

    for (const auto& entry : fs::directory_iterator(dir)) {
      if (entry.path().extension() == ".milk") {
        PresetInfo info;
        info.filename = entry.path().filename().string();
        info.name = entry.path().stem().string();
        info.path = entry.path().string();
        presets_.push_back(info);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error scanning preset directory " << dir << ": " << e.what() << "\n";
  }
}

bool PresetManager::scanPresets() {
  presets_.clear();
  currentIndex_ = 0;

  for (const auto& path : getPresetSearchPaths()) {
    scanDirectory(path);
  }

  if (presets_.empty()) {
    std::cout << "No presets found\n";
    return false;
  }

  std::cout << "Found " << presets_.size() << " presets\n";
  return true;
}

std::string PresetManager::loadPreset(const std::string& presetPath) {
  try {
    std::ifstream file(presetPath);
    if (!file.is_open()) {
      std::cerr << "Failed to open preset: " << presetPath << "\n";
      return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    std::cout << "Loaded preset: " << presetPath << "\n";
    return content;
  } catch (const std::exception& e) {
    std::cerr << "Error loading preset: " << e.what() << "\n";
    return "";
  }
}

std::string PresetManager::loadPresetByIndex(int index) {
  if (index < 0 || index >= (int)presets_.size()) {
    std::cerr << "Invalid preset index: " << index << "\n";
    return "";
  }

  currentIndex_ = index;
  return loadPreset(presets_[index].path);
}

void PresetManager::nextPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ + 1) % presets_.size();
}

void PresetManager::previousPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ - 1 + presets_.size()) % presets_.size();
}
