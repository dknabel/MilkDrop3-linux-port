// src/audio/pipewire_input.h
#pragma once
#include "../platform/audio.h"
#include <pipewire/pipewire.h>
#include <spa/pod/event.h>
#include <deque>
#include <mutex>

class PipeWireInput : public AudioInput {
public:
  PipeWireInput();
  ~PipeWireInput();

  bool initialize(const std::string& deviceName) override;
  void shutdown() override;

  bool getAudioFrame(AudioFrame& outFrame) override;
  std::vector<std::string> listDevices() const override;
  std::string getDefaultDevice() const override;

private:
  pw_main_loop* mainLoop_;
  pw_context* context_;
  pw_core* core_;
  pw_stream* stream_;

  std::deque<AudioFrame> audioBuffer_;
  std::mutex bufferLock_;

  bool isInitialized_;

  static void onStreamProcess(void* userData);
  static void onStreamStateChange(void* userData, pw_stream_state old, pw_stream_state state, const char* error);
};
