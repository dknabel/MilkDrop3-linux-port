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

  bool isConnected() const override { return isConnected_ && stream_; }
  bool attemptReconnect() override;
  int getReconnectAttempts() const override { return reconnectAttempts_; }

  // Static callbacks (public for stream_events initialization)
  static void onStreamProcess(void* userData);
  static void onStreamStateChange(void* userData, pw_stream_state old, pw_stream_state state, const char* error);

private:
  pw_main_loop* mainLoop_;
  pw_context* context_;
  pw_core* core_;
  pw_stream* stream_;

  std::deque<AudioFrame> audioBuffer_;
  std::mutex bufferLock_;

  bool isInitialized_;
  int reconnectAttempts_ = 0;
  static constexpr int MAX_RECONNECT_ATTEMPTS = 5;
  bool isConnected_ = false;
};
