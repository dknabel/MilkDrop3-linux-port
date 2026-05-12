// src/platform/config.h
#pragma once
#include <string>
#include "types.h"

class ConfigManager {
public:
  static ConfigManager& getInstance();

  bool load();
  bool save();

  Config& getConfig();
  const Config& getConfig() const;

private:
  ConfigManager() = default;
  ~ConfigManager() = default;

  Config config_;
  std::string configPath_;

  std::string getConfigDir() const;
};
