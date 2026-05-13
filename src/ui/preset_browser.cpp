#include "preset_browser.h"
#include <algorithm>
#include <iostream>
#include <cstdlib>

PresetBrowser::PresetBrowser() : currentIndex_(0) {}

bool PresetBrowser::scanPresets(const std::string& defaultPath) {
  // Expand ~ to home directory
  std::string expandedPath = defaultPath;
  if (expandedPath[0] == '~') {
    const char* home = std::getenv("HOME");
    if (home) {
      expandedPath = std::string(home) + expandedPath.substr(1);
    }
  }

  basePath_ = expandedPath;
  std::filesystem::path presetDir(basePath_);

  if (!std::filesystem::exists(presetDir)) {
    std::cerr << "Preset directory not found: " << basePath_ << "\n";
    return false;
  }

  presets_.clear();
  return loadPresetsFromDirectory(presetDir);
}

bool PresetBrowser::loadPresetsFromDirectory(const std::filesystem::path& dir) {
  try {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
      std::string ext = entry.path().extension().string();
      if (ext == ".milk" || ext == ".milk2") {
        presets_.push_back(entry.path().filename().string());
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error scanning preset directory: " << e.what() << "\n";
    return false;
  }

  std::sort(presets_.begin(), presets_.end());

  if (!presets_.empty()) {
    std::cout << "Loaded " << presets_.size() << " presets\n";
    return true;
  }

  return false;
}

void PresetBrowser::nextPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ + 1) % presets_.size();
}

void PresetBrowser::previousPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ - 1 + presets_.size()) % presets_.size();
}

std::string PresetBrowser::getCurrentPresetPath() const {
  if (currentIndex_ < presets_.size()) {
    return (std::filesystem::path(basePath_) / presets_[currentIndex_]).string();
  }
  return "";
}

std::string PresetBrowser::getCurrentPresetName() const {
  if (currentIndex_ < presets_.size()) {
    return presets_[currentIndex_];
  }
  return "";
}
