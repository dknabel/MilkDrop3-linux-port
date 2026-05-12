// src/ui/preset_manager.h
#pragma once
#include <string>
#include <vector>

struct PresetInfo {
  std::string filename;
  std::string name;
  std::string path;
};

class PresetManager {
public:
  PresetManager();

  // Scan and discover presets
  bool scanPresets();

  // Get available presets
  const std::vector<PresetInfo>& getPresets() const { return presets_; }

  // Load preset file
  std::string loadPreset(const std::string& presetPath);
  std::string loadPresetByIndex(int index);

  // Navigate
  int getCurrentPresetIndex() const { return currentIndex_; }
  void nextPreset();
  void previousPreset();

  // Get paths
  std::vector<std::string> getPresetSearchPaths() const;

private:
  std::vector<PresetInfo> presets_;
  int currentIndex_;

  void scanDirectory(const std::string& dir);
};
