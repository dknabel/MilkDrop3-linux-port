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

    // Create sample spec
    pa_sample_spec spec;
    spec.format = PA_SAMPLE_FLOAT32LE;
    spec.rate = 48000;
    spec.channels = 2;

    // Create stream
    stream_ = pa_stream_new(context_, "milkdrop3-input", &spec, nullptr);
    if (!stream_) {
      std::cerr << "Failed to create PulseAudio stream\n";
      pa_context_disconnect(context_);
      pa_context_unref(context_);
      pa_threaded_mainloop_stop(threadedMainLoop_);
      pa_threaded_mainloop_free(threadedMainLoop_);
      threadedMainLoop_ = nullptr;
      context_ = nullptr;
      return false;
    }

    // Set callbacks
    pa_stream_set_read_callback(stream_, onStreamReadCallback, this);
    pa_stream_set_state_callback(stream_, onStreamStateCallback, this);

    // Connect recording stream to monitor of default output
    if (pa_stream_connect_record(stream_, "@DEFAULT_MONITOR@", nullptr, PA_STREAM_ADJUST_LATENCY) < 0) {
      std::cerr << "Failed to connect recording stream\n";
      pa_stream_disconnect(stream_);
      pa_stream_unref(stream_);
      pa_context_disconnect(context_);
      pa_context_unref(context_);
      pa_threaded_mainloop_stop(threadedMainLoop_);
      pa_threaded_mainloop_free(threadedMainLoop_);
      stream_ = nullptr;
      threadedMainLoop_ = nullptr;
      context_ = nullptr;
      return false;
    }

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

bool PulseAudioInput::attemptReconnect() {
  if (reconnectAttempts_ >= MAX_RECONNECT_ATTEMPTS) {
    std::cerr << "Max reconnection attempts reached\n";
    return false;
  }

  reconnectAttempts_++;
  std::cout << "PulseAudio reconnection attempt " << reconnectAttempts_ << "\n";

  // Cleanup old stream
  if (stream_) {
    pa_stream_disconnect(stream_);
    pa_stream_unref(stream_);
    stream_ = nullptr;
  }

  // Try to reinitialize
  if (initialize("default")) {
    reconnectAttempts_ = 0;
    return true;
  }

  return false;
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

void PulseAudioInput::onStreamStateCallback(pa_stream* p, void* userdata) {
  PulseAudioInput* self = reinterpret_cast<PulseAudioInput*>(userdata);
  if (!self) return;

  pa_stream_state state = pa_stream_get_state(p);
  switch (state) {
    case PA_STREAM_READY:
      self->isConnected_ = true;
      std::cout << "PulseAudio stream ready\n";
      break;
    case PA_STREAM_FAILED:
      self->isConnected_ = false;
      std::cerr << "PulseAudio stream failed: " << pa_strerror(pa_context_errno(pa_stream_get_context(p))) << "\n";
      break;
    case PA_STREAM_TERMINATED:
      self->isConnected_ = false;
      std::cout << "PulseAudio stream terminated\n";
      break;
    default:
      break;
  }
}

void PulseAudioInput::onStreamReadCallback(pa_stream* p, size_t nbytes, void* userdata) {
  PulseAudioInput* self = static_cast<PulseAudioInput*>(userdata);
  const void* data;
  size_t bytes;

  while (pa_stream_peek(p, &data, &bytes) == 0 && bytes > 0) {
    if (data) {
      // Extract float samples
      const float* floatSamples = static_cast<const float*>(data);
      size_t sampleCount = bytes / sizeof(float);

      // Create audio frame
      AudioFrame frame;
      frame.samples.assign(floatSamples, floatSamples + sampleCount);
      frame.sampleRate = 48000;
      frame.channels = 2;

      // Queue in buffer with limit
      {
        std::lock_guard<std::mutex> lock(self->bufferLock_);
        if (self->audioBuffer_.size() < 10) {
          self->audioBuffer_.push_back(frame);
        }
      }
    }

    pa_stream_drop(p);
  }
}
