// src/audio/pulseaudio_input.h
#pragma once
#include "../platform/audio.h"
#include <pulse/pulseaudio.h>
#include <deque>
#include <mutex>

class PulseAudioInput : public AudioInput {
public:
  PulseAudioInput();
  ~PulseAudioInput();

  bool initialize(const std::string& deviceName) override;
  void shutdown() override;

  bool getAudioFrame(AudioFrame& outFrame) override;
  std::vector<std::string> listDevices() const override;
  std::string getDefaultDevice() const override;

private:
  pa_mainloop* mainLoop_;
  pa_context* context_;
  pa_stream* stream_;
  pa_threaded_mainloop* threadedMainLoop_;

  std::deque<AudioFrame> audioBuffer_;
  std::mutex bufferLock_;

  bool isInitialized_;

  static void onContextStateCallback(pa_context* c, void* userdata);
  static void onStreamReadCallback(pa_stream* p, size_t nbytes, void* userdata);
};
