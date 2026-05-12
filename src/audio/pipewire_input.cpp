// src/audio/pipewire_input.cpp
#include "pipewire_input.h"
#include <iostream>
#include <cstring>
#include <spa/param/audio/format.h>
#include <spa/pod/builder.h>

static const pw_stream_events stream_events = {
  .version = PW_VERSION_STREAM_EVENTS,
  .process = PipeWireInput::onStreamProcess,
  .state_changed = PipeWireInput::onStreamStateChange,
};

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
      pw_main_loop_destroy(mainLoop_);
      mainLoop_ = nullptr;
      return false;
    }

    // Connect to core
    core_ = pw_context_connect(context_, nullptr, 0);
    if (!core_) {
      std::cerr << "Failed to connect to PipeWire core\n";
      pw_context_destroy(context_);
      context_ = nullptr;
      pw_main_loop_destroy(mainLoop_);
      mainLoop_ = nullptr;
      return false;
    }

    // Create recording stream
    uint8_t buffer[1024];
    spa_audio_info_raw audio_info = SPA_AUDIO_INFO_RAW_INIT(
      .format = SPA_AUDIO_FORMAT_F32LE,
      .channels = 2,
      .rate = 48000
    );

    const spa_pod *params[1];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &audio_info);

    stream_ = pw_stream_new_simple(
      pw_main_loop_get_loop(mainLoop_),
      "milkdrop3-visualizer",
      pw_properties_new(
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Playback",
        NULL
      ),
      &stream_events,
      this
    );

    pw_stream_connect(stream_, PW_DIRECTION_INPUT, PW_ID_ANY, PW_STREAM_FLAG_AUTOCONNECT, params, 1);

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

bool PipeWireInput::attemptReconnect() {
  if (reconnectAttempts_ >= MAX_RECONNECT_ATTEMPTS) {
    std::cerr << "Max reconnection attempts reached\n";
    return false;
  }

  reconnectAttempts_++;
  std::cout << "PipeWire reconnection attempt " << reconnectAttempts_ << "\n";

  // Cleanup old stream
  if (stream_) {
    pw_stream_disconnect(stream_);
    pw_stream_destroy(stream_);
    stream_ = nullptr;
  }

  // Try to reinitialize
  if (initialize("default")) {
    reconnectAttempts_ = 0;
    return true;
  }

  return false;
}

void PipeWireInput::onStreamProcess(void* userData) {
  PipeWireInput* self = reinterpret_cast<PipeWireInput*>(userData);
  if (!self || !self->stream_) return;

  pw_buffer* pwBuf = pw_stream_dequeue_buffer(self->stream_);
  if (!pwBuf) return;

  spa_buffer* buffer = pwBuf->buffer;
  if (buffer->datas[0].data) {
    float* samples = (float*)buffer->datas[0].data;
    uint32_t n_samples = buffer->datas[0].chunk->size / sizeof(float);

    // Queue audio frame
    AudioFrame frame;
    frame.samples.resize(n_samples);
    std::copy(samples, samples + n_samples, frame.samples.begin());
    frame.sampleRate = 48000;
    frame.channels = 2;

    {
      std::lock_guard<std::mutex> lock(self->bufferLock_);
      self->audioBuffer_.push_back(frame);
      // Keep buffer size reasonable
      if (self->audioBuffer_.size() > 10) {
        self->audioBuffer_.pop_front();
      }
    }
  }

  pw_stream_queue_buffer(self->stream_, pwBuf);
}

void PipeWireInput::onStreamStateChange(void* userData, pw_stream_state old,
                                       pw_stream_state state, const char* error) {
  PipeWireInput* self = reinterpret_cast<PipeWireInput*>(userData);
  if (!self) return;

  if (error) {
    std::cerr << "PipeWire stream error: " << error << "\n";
  }

  switch (state) {
    case PW_STREAM_STATE_STREAMING:
      self->isConnected_ = true;
      std::cout << "PipeWire stream connected\n";
      break;
    case PW_STREAM_STATE_PAUSED:
      self->isConnected_ = false;
      std::cout << "PipeWire stream paused\n";
      break;
    case PW_STREAM_STATE_ERROR:
      self->isConnected_ = false;
      std::cerr << "PipeWire stream error state\n";
      break;
    default:
      break;
  }
}
