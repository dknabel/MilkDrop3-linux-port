# Phase 4.2: Audio Callbacks Implementation Plan

> **For agentic workers:** Use superpowers:subagent-driven-development to execute this plan task-by-task.

**Goal:** Complete audio capture callbacks in PipeWire and PulseAudio implementations. Enable system audio → buffer pipeline.

**Architecture:** Implement stream processing callbacks that extract audio frames and queue them for analysis.

**Tech Stack:** PipeWire 0.3+, PulseAudio 13+, std::deque for thread-safe buffering

---

## File Structure

**To be modified:**
```
src/audio/
  ├── pipewire_input.cpp    (complete onStreamProcess callback)
  └── pulseaudio_input.cpp  (complete onStreamReadCallback)
```

No new files. Focus on completing TODO callbacks.

---

## Phase 4.2 Tasks

### Task 1: Complete PipeWire Stream Callbacks

**Files:**
- Modify: `src/audio/pipewire_input.h` - Update callback signatures
- Modify: `src/audio/pipewire_input.cpp` - Implement onStreamProcess and stream creation

**Specification:**

In `pipewire_input.h`, update callback method signatures:

```cpp
private:
  static void onStreamProcess(void* userData);
  static void onStreamStateChange(void* userData, pw_stream_state old, 
                                  pw_stream_state state, const char* error);
```

In `pipewire_input.cpp`, complete stream creation in `initialize()` method (after core connection):

```cpp
// Create recording stream
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
```

Implement `onStreamProcess()` callback:

```cpp
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
```

Implement `onStreamStateChange()` callback:

```cpp
void PipeWireInput::onStreamStateChange(void* userData, pw_stream_state old,
                                        pw_stream_state state, const char* error) {
  if (error) {
    std::cerr << "PipeWire stream error: " << error << "\n";
  }
  
  switch (state) {
    case PW_STREAM_STATE_STREAMING:
      std::cout << "PipeWire stream streaming\n";
      break;
    case PW_STREAM_STATE_PAUSED:
      std::cout << "PipeWire stream paused\n";
      break;
    case PW_STREAM_STATE_ERROR:
      std::cerr << "PipeWire stream error state\n";
      break;
    default:
      break;
  }
}
```

Add stream_events structure in cpp file (global or static):

```cpp
static const pw_stream_events stream_events = {
  .version = PW_VERSION_STREAM_EVENTS,
  .process = PipeWireInput::onStreamProcess,
  .state_changed = PipeWireInput::onStreamStateChange,
};
```

---

### Task 2: Complete PulseAudio Stream Callbacks

**Files:**
- Modify: `src/audio/pulseaudio_input.h` - Update callback signatures  
- Modify: `src/audio/pulseaudio_input.cpp` - Implement stream creation and callbacks

**Specification:**

In `pulseaudio_input.cpp`, complete stream creation in `initialize()` method (after context ready):

```cpp
// Create recording stream
pa_sample_spec ss = {
  .format = PA_SAMPLE_FLOAT32LE,
  .rate = 48000,
  .channels = 2
};

stream_ = pa_stream_new(context_, "milkdrop3-visualizer", &ss, nullptr);
pa_stream_set_read_callback(stream_, onStreamReadCallback, this);
pa_stream_set_state_callback(stream_, onStreamStateCallback, this);

// Connect to monitor of default sink (system audio)
pa_stream_connect_record(
  stream_,
  "@DEFAULT_MONITOR@",
  nullptr,
  (pa_stream_flags_t)(PA_STREAM_DONT_MOVE | PA_STREAM_ADJUST_LATENCY)
);
```

Implement `onStreamReadCallback()`:

```cpp
void PulseAudioInput::onStreamReadCallback(pa_stream* p, size_t nbytes, void* userdata) {
  PulseAudioInput* self = reinterpret_cast<PulseAudioInput*>(userdata);
  if (!self || !p) return;
  
  const void* data = nullptr;
  if (pa_stream_peek(p, &data, &nbytes) < 0) {
    std::cerr << "PulseAudio peek error: " << pa_strerror(pa_context_errno(self->context_)) << "\n";
    return;
  }
  
  if (data && nbytes > 0) {
    float* samples = (float*)data;
    uint32_t n_samples = nbytes / sizeof(float);
    
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
  
  pa_stream_drop(p);
}
```

Implement `onStreamStateCallback()` (already partially done):

```cpp
void PulseAudioInput::onStreamStateCallback(pa_stream* p, void* userdata) {
  PulseAudioInput* self = reinterpret_cast<PulseAudioInput*>(userdata);
  
  pa_stream_state state = pa_stream_get_state(p);
  switch (state) {
    case PA_STREAM_READY:
      std::cout << "PulseAudio stream ready\n";
      break;
    case PA_STREAM_FAILED:
      std::cerr << "PulseAudio stream failed: " << pa_strerror(pa_context_errno(self->context_)) << "\n";
      break;
    case PA_STREAM_TERMINATED:
      std::cout << "PulseAudio stream terminated\n";
      break;
    default:
      break;
  }
}
```

---

### Task 3: Update AudioInput Factory for Stream Testing

**Files:**
- Modify: `src/audio/factory.cpp` - Add logging for successful initialization

**Specification:**

Update `createAudioInput()` to test stream connection:

```cpp
AudioInput* createAudioInput() {
#ifdef HAVE_PIPEWIRE
  std::cout << "Creating PipeWire audio input...\n";
  auto* input = new PipeWireInput();
  if (input->initialize("default")) {
    std::cout << "PipeWire initialized successfully\n";
    return input;
  }
  std::cerr << "PipeWire initialization failed, trying PulseAudio...\n";
  delete input;
#endif

#ifdef HAVE_PULSEAUDIO
  std::cout << "Creating PulseAudio audio input...\n";
  auto* input = new PulseAudioInput();
  if (input->initialize("default")) {
    std::cout << "PulseAudio initialized successfully\n";
    return input;
  }
  std::cerr << "PulseAudio initialization failed\n";
  delete input;
#endif

  std::cerr << "No audio backend available\n";
  return nullptr;
}
```

---

## Success Criteria

- [ ] PipeWire stream created and connected
- [ ] PipeWire callbacks receive audio frames
- [ ] PulseAudio stream created and connected
- [ ] PulseAudio callbacks receive audio frames
- [ ] AudioInput::getAudioFrame() returns non-empty frames
- [ ] Thread safety maintained (mutex protecting buffer)
- [ ] Code compiles without errors
- [ ] Buffer doesn't grow unbounded (keep last 10 frames max)

---

## Notes

- Audio format: F32LE (32-bit float, little-endian) at 48kHz, 2 channels
- PipeWire: Uses low-level pw_stream API with callback registration
- PulseAudio: Uses pa_stream_peek/drop pattern for recording
- Both maintain thread-safe FIFO buffers with size limits
- Error messages logged to help diagnose audio device issues

