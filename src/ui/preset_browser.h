#pragma once
#include <string>
#include <vector>
#include <filesystem>

class PresetBrowser {
public:
  PresetBrowser();

  // Scan preset directories and build list
  bool scanPresets(const std::string& defaultPath = "~/.milkdrop3/presets");

  // Get current preset list
  const std::vector<std::string>& getPresets() const { return presets_; }

  // Get current selection index
  size_t getCurrentIndex() const { return currentIndex_; }

  // Navigate presets
  void nextPreset();
  void previousPreset();

  // Get full path to current preset
  std::string getCurrentPresetPath() const;
  std::string getCurrentPresetName() const;

private:
  std::vector<std::string> presets_;
  std::string basePath_;
  size_t currentIndex_;

  bool loadPresetsFromDirectory(const std::filesystem::path& dir);
};
