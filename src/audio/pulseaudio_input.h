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

  bool isConnected() const override { return isConnected_ && stream_; }
  bool attemptReconnect() override;
  int getReconnectAttempts() const override { return reconnectAttempts_; }

private:
  pa_mainloop_api* mainLoop_;
  pa_context* context_;
  pa_stream* stream_;
  pa_threaded_mainloop* threadedMainLoop_;

  std::deque<AudioFrame> audioBuffer_;
  std::mutex bufferLock_;

  bool isInitialized_;
  int reconnectAttempts_ = 0;
  static constexpr int MAX_RECONNECT_ATTEMPTS = 5;
  bool isConnected_ = false;

  static void onContextStateCallback(pa_context* c, void* userdata);
  static void onStreamStateCallback(pa_stream* p, void* userdata);
  static void onStreamReadCallback(pa_stream* p, size_t nbytes, void* userdata);
};
