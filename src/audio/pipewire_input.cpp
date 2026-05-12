// src/audio/pipewire_input.cpp
#include "pipewire_input.h"
#include <iostream>
#include <cstring>

PipeWireInput::PipeWireInput()
  : mainLoop_(nullptr), context_(nullptr), core_(nullptr),
    stream_(nullptr), isInitialized_(false) {
  pw_init(nullptr, nullptr);
}

PipeWireInput::~PipeWireInput() {
  shutdown();
  pw_deinit();
}

bool PipeWireInput::initialize(const std::string& deviceName) {
  try {
    // Create main loop
    mainLoop_ = pw_main_loop_new(nullptr);
    if (!mainLoop_) {
      std::cerr << "Failed to create PipeWire main loop\n";
      return false;
    }

    // Create context
    context_ = pw_context_new(pw_main_loop_get_loop(mainLoop_), nullptr, 0);
    if (!context_) {
      std::cerr << "Failed to create PipeWire context\n";
      return false;
    }

    // Connect to core
    core_ = pw_context_connect(context_, nullptr, 0);
    if (!core_) {
      std::cerr << "Failed to connect to PipeWire core\n";
      return false;
    }

    // TODO: Create stream with device selection
    // This is a simplified version; full implementation will handle:
    // - Device enumeration and selection
    // - Stream configuration (sample rate, format, channels)
    // - Callbacks for processing audio data

    isInitialized_ = true;
    std::cout << "PipeWire audio input initialized\n";
    return true;
  } catch (const std::exception& e) {
    std::cerr << "PipeWire initialization error: " << e.what() << "\n";
    return false;
  }
}

void PipeWireInput::shutdown() {
  if (stream_) {
    pw_stream_destroy(stream_);
    stream_ = nullptr;
  }

  if (core_) {
    pw_core_disconnect(core_);
    core_ = nullptr;
  }

  if (context_) {
    pw_context_destroy(context_);
    context_ = nullptr;
  }

  if (mainLoop_) {
    pw_main_loop_destroy(mainLoop_);
    mainLoop_ = nullptr;
  }

  isInitialized_ = false;
}

bool PipeWireInput::getAudioFrame(AudioFrame& outFrame) {
  std::lock_guard<std::mutex> lock(bufferLock_);

  if (audioBuffer_.empty()) {
    return false;
  }

  outFrame = audioBuffer_.front();
  audioBuffer_.pop_front();
  return true;
}

std::vector<std::string> PipeWireInput::listDevices() const {
  std::vector<std::string> devices;
  // TODO: Enumerate PipeWire devices
  devices.push_back("default");
  return devices;
}

std::string PipeWireInput::getDefaultDevice() const {
  return "default";
}

void PipeWireInput::onStreamProcess(void* userData) {
  // PipeWire callback for processing audio
  // TODO: Extract audio frames and queue them
}

void PipeWireInput::onStreamStateChange(void* userData, pw_stream_state old,
                                       pw_stream_state state, const char* error) {
  if (error) {
    std::cerr << "PipeWire stream state change error: " << error << "\n";
  }
}
