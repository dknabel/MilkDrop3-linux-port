// src/platform/audio.h
#pragma once
#include <string>
#include <vector>
#include "types.h"

class AudioInput {
public:
  virtual ~AudioInput() = default;

  virtual bool initialize(const std::string& deviceName) = 0;
  virtual void shutdown() = 0;

  virtual bool getAudioFrame(AudioFrame& outFrame) = 0;
  virtual std::vector<std::string> listDevices() const = 0;
  virtual std::string getDefaultDevice() const = 0;
};

// Factory function
AudioInput* createAudioInput();
