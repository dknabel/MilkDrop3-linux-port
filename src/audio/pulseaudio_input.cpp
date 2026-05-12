// src/audio/pulseaudio_input.cpp
#include "pulseaudio_input.h"
#include <iostream>
#include <cstring>

PulseAudioInput::PulseAudioInput()
  : mainLoop_(nullptr), context_(nullptr), stream_(nullptr),
    threadedMainLoop_(nullptr), isInitialized_(false) {
}

PulseAudioInput::~PulseAudioInput() {
  shutdown();
}

bool PulseAudioInput::initialize(const std::string& deviceName) {
  try {
    // Create threaded main loop
    threadedMainLoop_ = pa_threaded_mainloop_new();
    if (!threadedMainLoop_) {
      std::cerr << "Failed to create PulseAudio main loop\n";
      return false;
    }

    mainLoop_ = pa_threaded_mainloop_get_api(threadedMainLoop_);

    // Create context
    context_ = pa_context_new(mainLoop_, "milkdrop3");
    if (!context_) {
      std::cerr << "Failed to create PulseAudio context\n";
      return false;
    }

    // Set context state callback
    pa_context_set_state_callback(context_, onContextStateCallback, this);

    // Connect to server
    if (pa_context_connect(context_, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr) < 0) {
      std::cerr << "Failed to connect to PulseAudio server\n";
      return false;
    }

    // Start main loop
    if (pa_threaded_mainloop_start(threadedMainLoop_) < 0) {
      std::cerr << "Failed to start PulseAudio main loop\n";
      pa_context_disconnect(context_);
      pa_context_unref(context_);
      pa_threaded_mainloop_free(threadedMainLoop_);
      threadedMainLoop_ = nullptr;
      context_ = nullptr;
      return false;
    }

    // TODO: Create and connect recording stream
    // - Specify sample rate (44100 Hz common for visualization)
    // - Format: PCM 16-bit or 32-bit float
    // - Channels: mono or stereo

    isInitialized_ = true;
    std::cout << "PulseAudio input initialized\n";
    return true;
  } catch (const std::exception& e) {
    std::cerr << "PulseAudio initialization error: " << e.what() << "\n";
    return false;
  }
}

void PulseAudioInput::shutdown() {
  if (stream_) {
    pa_stream_disconnect(stream_);
    pa_stream_unref(stream_);
    stream_ = nullptr;
  }

  if (context_) {
    pa_context_disconnect(context_);
    pa_context_unref(context_);
    context_ = nullptr;
  }

  if (threadedMainLoop_) {
    pa_threaded_mainloop_stop(threadedMainLoop_);
    pa_threaded_mainloop_free(threadedMainLoop_);
    threadedMainLoop_ = nullptr;
  }

  isInitialized_ = false;
}

bool PulseAudioInput::getAudioFrame(AudioFrame& outFrame) {
  std::lock_guard<std::mutex> lock(bufferLock_);

  if (audioBuffer_.empty()) {
    return false;
  }

  outFrame = audioBuffer_.front();
  audioBuffer_.pop_front();
  return true;
}

std::vector<std::string> PulseAudioInput::listDevices() const {
  std::vector<std::string> devices;
  // TODO: Enumerate PulseAudio recording devices
  devices.push_back("default");
  return devices;
}

std::string PulseAudioInput::getDefaultDevice() const {
  return "default";
}

void PulseAudioInput::onContextStateCallback(pa_context* c, void* userdata) {
  pa_context_state state = pa_context_get_state(c);

  switch (state) {
    case PA_CONTEXT_READY:
      std::cout << "PulseAudio context ready\n";
      break;
    case PA_CONTEXT_FAILED:
      std::cerr << "PulseAudio context failed\n";
      break;
    case PA_CONTEXT_TERMINATED:
      std::cout << "PulseAudio context terminated\n";
      break;
    default:
      break;
  }
}

void PulseAudioInput::onStreamReadCallback(pa_stream* p, size_t nbytes, void* userdata) {
  // Called when audio data is available
  // TODO: Read samples and queue them for processing
}
