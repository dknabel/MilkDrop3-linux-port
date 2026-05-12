# Milkdrop3 Linux Port Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port Milkdrop3 audio visualizer from Windows to Linux while maintaining 100% preset compatibility.

**Architecture:** Build platform abstraction layers (audio, windowing, graphics) that replace Windows-specific code with Linux equivalents. Keep the core visualization engine unchanged from Milkdrop3 source.

**Tech Stack:** C++17, CMake, GLFW3, OpenGL 3.3+, PipeWire/PulseAudio, GLM

---

## File Structure

**To be created:**
```
src/
├── main.cpp                                 (entry point, main loop)
├── platform/
│   ├── audio.h                              (AudioInput abstract interface)
│   ├── window.h                             (Window abstract interface)
│   ├── graphics.h                           (GraphicsDevice abstract interface)
│   ├── config.h / config.cpp               (configuration management, XDG paths)
│   └── types.h                              (shared types, InputState, etc.)
├── audio/
│   ├── pipewire_input.h / pipewire_input.cpp
│   ├── pulseaudio_input.h / pulseaudio_input.cpp
│   └── audio_analyzer.h / audio_analyzer.cpp (FFT, frequency analysis)
├── graphics/
│   ├── opengl_device.h / opengl_device.cpp
│   ├── shader.h / shader.cpp                (shader compilation, error handling)
│   └── render_command.h                     (render command structure)
├── window/
│   ├── glfw_window.h / glfw_window.cpp
│   └── input.h                              (input handling, keyboard/mouse)
├── ui/
│   ├── preset_manager.h / preset_manager.cpp (preset discovery, loading)
│   └── overlay.h / overlay.cpp              (simple text overlay for info)
├── core/                                    (from Milkdrop3 source, integrated)
│   └── visualization.h / visualization.cpp
└── CMakeLists.txt

CMakeLists.txt (root)
```

---

## Phase 1: Setup & Build System

### Task 1: Initialize CMake Build System

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/CMakeLists.txt`
- Create: `.gitignore`

- [ ] **Step 1: Create root CMakeLists.txt**

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(Milkdrop3 CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE libpipewire-0.3)
pkg_check_modules(PULSEAUDIO libpulse)

if(NOT PIPEWIRE_FOUND AND NOT PULSEAUDIO_FOUND)
  message(FATAL_ERROR "Neither PipeWire nor PulseAudio development files found")
endif()

# GLM header-only
find_package(GLM REQUIRED)

# Subdirectory
add_subdirectory(src)

# Install targets
install(TARGETS milkdrop3 RUNTIME DESTINATION bin)
```

- [ ] **Step 2: Create src/CMakeLists.txt**

```cmake
# src/CMakeLists.txt
set(MILKDROP_SOURCES
  main.cpp
  platform/config.cpp
  platform/types.h
  audio/audio_analyzer.cpp
  graphics/shader.cpp
  window/input.h
  ui/preset_manager.cpp
  ui/overlay.cpp
)

# Platform-specific sources
if(PIPEWIRE_FOUND)
  list(APPEND MILKDROP_SOURCES audio/pipewire_input.cpp)
endif()

if(PULSEAUDIO_FOUND)
  list(APPEND MILKDROP_SOURCES audio/pulseaudio_input.cpp)
endif()

list(APPEND MILKDROP_SOURCES
  window/glfw_window.cpp
  graphics/opengl_device.cpp
  core/visualization.cpp
)

add_executable(milkdrop3 ${MILKDROP_SOURCES})

target_include_directories(milkdrop3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(milkdrop3
  OpenGL::OpenGL
  glfw
  glm::glm
)

if(PIPEWIRE_FOUND)
  target_link_libraries(milkdrop3 ${PIPEWIRE_LIBRARIES})
  target_include_directories(milkdrop3 PRIVATE ${PIPEWIRE_INCLUDE_DIRS})
  target_compile_definitions(milkdrop3 PRIVATE HAVE_PIPEWIRE)
endif()

if(PULSEAUDIO_FOUND)
  target_link_libraries(milkdrop3 ${PULSEAUDIO_LIBRARIES})
  target_include_directories(milkdrop3 PRIVATE ${PULSEAUDIO_INCLUDE_DIRS})
  target_compile_definitions(milkdrop3 PRIVATE HAVE_PULSEAUDIO)
endif()
```

- [ ] **Step 3: Create .gitignore**

```
build/
*.o
*.a
*.so
*.dylib
.DS_Store
*.swp
*.swo
*~
.vscode/
.idea/
CMakeFiles/
cmake_install.cmake
Makefile
milkdrop3
```

- [ ] **Step 4: Test CMake configuration**

```bash
cd /home/drew/Documents/molkdroop
mkdir build
cd build
cmake ..
```

Expected output: CMake should find GLFW3, OpenGL, and at least one of PipeWire/PulseAudio. If it fails to find audio libraries, document the issue — this is expected on systems without dev packages installed.

- [ ] **Step 5: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add CMakeLists.txt src/CMakeLists.txt .gitignore
git commit -m "build: add CMake build system

- Root and src CMakeLists.txt with dependency detection
- Support for both PipeWire and PulseAudio
- OpenGL, GLFW3, GLM as required dependencies
- .gitignore for build artifacts

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

## Phase 1: Platform Abstraction Interfaces

### Task 2: Define Platform Types & Shared Interfaces

**Files:**
- Create: `src/platform/types.h`
- Create: `src/platform/audio.h`
- Create: `src/platform/window.h`
- Create: `src/platform/graphics.h`

- [ ] **Step 1: Create platform/types.h**

```cpp
// src/platform/types.h
#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct InputState {
  bool key_escape;
  bool key_p;           // pause/play
  bool key_f;           // fullscreen toggle
  bool key_s;           // show preset list
  bool key_left;        // previous preset
  bool key_right;       // next preset
  bool key_up;          // parameter up
  bool key_down;        // parameter down
};

struct AudioFrame {
  std::vector<float> samples;
  int sampleRate;
  int channels;
};

enum class BlendMode {
  Replace,
  Add,
  Multiply,
  Alpha
};

struct RenderCommand {
  // Simplified render command for visualization
  // Full implementation will depend on Milkdrop3 core
  uint32_t shaderHandle;
  uint32_t vertexBufferHandle;
  uint32_t indexCount;
  BlendMode blendMode;
};

struct Config {
  std::string lastPreset;
  std::string audioDevice;
  int resolutionWidth = 1920;
  int resolutionHeight = 1080;
  bool fullscreen = false;
  float audioSensitivity = 1.0f;
};
```

- [ ] **Step 2: Create platform/audio.h**

```cpp
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
  virtual std::vector<std::string> listDevices() = 0;
  virtual std::string getDefaultDevice() = 0;
};

// Factory function
AudioInput* createAudioInput();
```

- [ ] **Step 3: Create platform/window.h**

```cpp
// src/platform/window.h
#pragma once
#include <string>
#include "types.h"

class Window {
public:
  virtual ~Window() = default;
  
  virtual bool create(int width, int height, bool fullscreen) = 0;
  virtual void shutdown() = 0;
  
  virtual bool update() = 0;
  virtual bool shouldClose() = 0;
  
  virtual void getInputState(InputState& outState) = 0;
  virtual void makeGLContextCurrent() = 0;
  virtual void swapBuffers() = 0;
  
  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
};

// Factory function
Window* createWindow();
```

- [ ] **Step 4: Create platform/graphics.h**

```cpp
// src/platform/graphics.h
#pragma once
#include <string>
#include <cstdint>
#include "types.h"

class Shader {
public:
  virtual ~Shader() = default;
  
  virtual uint32_t getHandle() const = 0;
  virtual bool isValid() const = 0;
  virtual std::string getErrorLog() const = 0;
};

class Texture {
public:
  virtual ~Texture() = default;
  
  virtual uint32_t getHandle() const = 0;
  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
};

class GraphicsDevice {
public:
  virtual ~GraphicsDevice() = default;
  
  virtual bool initialize(Window* window) = 0;
  virtual void shutdown() = 0;
  
  virtual Shader* createShader(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
  virtual void deleteShader(Shader* shader) = 0;
  
  virtual Texture* createTexture(int width, int height, const void* data) = 0;
  virtual void deleteTexture(Texture* texture) = 0;
  
  virtual void clear(float r, float g, float b, float a) = 0;
  virtual void executeRenderCommand(const RenderCommand& cmd) = 0;
  virtual void present() = 0;
};

// Factory function
GraphicsDevice* createGraphicsDevice();
```

- [ ] **Step 5: Create platform/config.h**

```cpp
// src/platform/config.h
#pragma once
#include <string>
#include "types.h"

class ConfigManager {
public:
  static ConfigManager& getInstance();
  
  bool load();
  bool save();
  
  Config& getConfig();
  const Config& getConfig() const;
  
private:
  ConfigManager() = default;
  ~ConfigManager() = default;
  
  Config config_;
  std::string configPath_;
  
  std::string getConfigDir() const;
};
```

- [ ] **Step 6: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/platform/
git commit -m "feat: define platform abstraction interfaces

- AudioInput: capture system audio, list devices
- Window: windowing, input handling, GL context
- GraphicsDevice: shader/texture management, rendering
- Shared types: InputState, AudioFrame, RenderCommand, Config
- ConfigManager: XDG-compliant config loading/saving

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

### Task 3: Implement ConfigManager (XDG Directories)

**Files:**
- Create: `src/platform/config.cpp`

- [ ] **Step 1: Implement ConfigManager**

```cpp
// src/platform/config.cpp
#include "config.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

ConfigManager& ConfigManager::getInstance() {
  static ConfigManager instance;
  return instance;
}

std::string ConfigManager::getConfigDir() const {
  // XDG Base Directory specification
  const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
  fs::path configDir;
  
  if (xdgConfig && fs::path(xdgConfig).is_absolute()) {
    configDir = fs::path(xdgConfig) / "milkdrop";
  } else {
    const char* home = std::getenv("HOME");
    if (!home) {
      throw std::runtime_error("HOME environment variable not set");
    }
    configDir = fs::path(home) / ".config" / "milkdrop";
  }
  
  // Create directory if it doesn't exist
  fs::create_directories(configDir);
  return configDir.string();
}

bool ConfigManager::load() {
  try {
    configPath_ = getConfigDir() + "/config.json";
    
    std::ifstream file(configPath_);
    if (!file.is_open()) {
      // First run, use defaults
      return true;
    }
    
    // TODO: Parse JSON (for MVP, use defaults)
    // Full implementation will use nlohmann/json or similar
    return true;
  } catch (const std::exception& e) {
    // Log and use defaults
    return true;
  }
}

bool ConfigManager::save() {
  try {
    configPath_ = getConfigDir() + "/config.json";
    
    std::ofstream file(configPath_);
    if (!file.is_open()) {
      return false;
    }
    
    // TODO: Serialize config to JSON
    // For MVP, write minimal JSON manually
    file << "{\n";
    file << "  \"lastPreset\": \"" << config_.lastPreset << "\",\n";
    file << "  \"audioDevice\": \"" << config_.audioDevice << "\",\n";
    file << "  \"resolutionWidth\": " << config_.resolutionWidth << ",\n";
    file << "  \"resolutionHeight\": " << config_.resolutionHeight << ",\n";
    file << "  \"fullscreen\": " << (config_.fullscreen ? "true" : "false") << ",\n";
    file << "  \"audioSensitivity\": " << config_.audioSensitivity << "\n";
    file << "}\n";
    
    return true;
  } catch (const std::exception& e) {
    return false;
  }
}

Config& ConfigManager::getConfig() {
  return config_;
}

const Config& ConfigManager::getConfig() const {
  return config_;
}
```

- [ ] **Step 2: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/platform/config.cpp
git commit -m "feat: implement ConfigManager with XDG Base Directory support

- Load/save config from ~/.config/milkdrop/config.json
- Create config directory if it doesn't exist
- Minimal JSON serialization (manual for MVP)
- Fallback to defaults on first run or errors

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

## Phase 1: Audio Implementation

### Task 4: Implement Audio Analyzer (FFT)

**Files:**
- Create: `src/audio/audio_analyzer.h`
- Create: `src/audio/audio_analyzer.cpp`

- [ ] **Step 1: Create audio/audio_analyzer.h**

```cpp
// src/audio/audio_analyzer.h
#pragma once
#include <vector>
#include <complex>

class AudioAnalyzer {
public:
  AudioAnalyzer(int fftSize = 512);
  
  // Input: raw audio samples
  // Output: frequency bins (magnitude spectrum)
  std::vector<float> analyze(const std::vector<float>& samples);
  
  int getFrequencyBinCount() const { return fftSize_ / 2; }
  
private:
  int fftSize_;
  std::vector<float> window_;
  std::vector<float> buffer_;
  
  void applyWindow(std::vector<float>& samples);
  void computeFFT(std::vector<std::complex<float>>& data);
};
```

- [ ] **Step 2: Create audio/audio_analyzer.cpp**

```cpp
// src/audio/audio_analyzer.cpp
#include "audio_analyzer.h"
#include <cmath>
#include <algorithm>

AudioAnalyzer::AudioAnalyzer(int fftSize)
  : fftSize_(fftSize) {
  
  // Hann window for FFT
  window_.resize(fftSize);
  for (int i = 0; i < fftSize; ++i) {
    window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize - 1)));
  }
  
  buffer_.resize(fftSize, 0.0f);
}

std::vector<float> AudioAnalyzer::analyze(const std::vector<float>& samples) {
  // Shift buffer and add new samples
  int samples_to_add = std::min((int)samples.size(), fftSize_);
  
  if (samples.size() < fftSize_) {
    // Pad with zeros if not enough samples
    std::copy(samples.begin(), samples.end(), buffer_.begin());
    std::fill(buffer_.begin() + samples.size(), buffer_.end(), 0.0f);
  } else {
    std::copy(samples.begin(), samples.begin() + fftSize_, buffer_.begin());
  }
  
  // Apply window
  applyWindow(buffer_);
  
  // Simple frequency magnitude estimation (placeholder)
  // Full implementation would use FFT library (FFTPACK, KissFFT, etc.)
  std::vector<float> freqBins(fftSize_ / 2);
  
  // For MVP: simple energy calculation per frequency band
  for (int i = 0; i < fftSize_ / 2; ++i) {
    int idx = (i * 2);
    if (idx < (int)buffer_.size()) {
      freqBins[i] = std::abs(buffer_[idx]) * window_[idx];
    }
  }
  
  return freqBins;
}

void AudioAnalyzer::applyWindow(std::vector<float>& samples) {
  for (int i = 0; i < (int)samples.size(); ++i) {
    samples[i] *= window_[i];
  }
}

void AudioAnalyzer::computeFFT(std::vector<std::complex<float>>& data) {
  // TODO: Implement proper FFT using FFTPACK or KissFFT library
  // This is a placeholder; real implementation will use a proven FFT library
}
```

- [ ] **Step 3: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/audio/audio_analyzer.*
git commit -m "feat: add audio analyzer with windowing

- Hann window for FFT preprocessing
- Placeholder frequency bin computation
- TODO: integrate real FFT library (FFTPACK/KissFFT)
- Accepts raw samples, outputs magnitude spectrum

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

### Task 5: Implement PipeWire Audio Input

**Files:**
- Create: `src/audio/pipewire_input.h`
- Create: `src/audio/pipewire_input.cpp`

- [ ] **Step 1: Create audio/pipewire_input.h**

```cpp
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
  std::vector<std::string> listDevices() override;
  std::string getDefaultDevice() override;
  
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
```

- [ ] **Step 2: Create audio/pipewire_input.cpp**

```cpp
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

std::vector<std::string> PipeWireInput::listDevices() {
  std::vector<std::string> devices;
  // TODO: Enumerate PipeWire devices
  devices.push_back("default");
  return devices;
}

std::string PipeWireInput::getDefaultDevice() {
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
```

- [ ] **Step 3: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/audio/pipewire_input.*
git commit -m "feat: implement PipeWire audio input (scaffold)

- PipeWire main loop, context, and core initialization
- Stream setup scaffold (TODO: full stream configuration)
- Device enumeration placeholder
- Audio frame buffering with thread-safe queue
- Callback structure for audio processing

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

### Task 6: Implement PulseAudio Audio Input (Fallback)

**Files:**
- Create: `src/audio/pulseaudio_input.h`
- Create: `src/audio/pulseaudio_input.cpp`

- [ ] **Step 1: Create audio/pulseaudio_input.h**

```cpp
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
  std::vector<std::string> listDevices() override;
  std::string getDefaultDevice() override;
  
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
```

- [ ] **Step 2: Create audio/pulseaudio_input.cpp**

```cpp
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

std::vector<std::string> PulseAudioInput::listDevices() {
  std::vector<std::string> devices;
  // TODO: Enumerate PulseAudio recording devices
  devices.push_back("default");
  return devices;
}

std::string PulseAudioInput::getDefaultDevice() {
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
```

- [ ] **Step 3: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/audio/pulseaudio_input.*
git commit -m "feat: implement PulseAudio audio input (scaffold)

- Threaded main loop for non-blocking operation
- Context and server connection
- Stream creation scaffold (TODO: full configuration)
- Device enumeration placeholder
- Audio frame buffering with thread-safe queue
- Context state and stream read callbacks

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

### Task 7: Create Audio Factory Function

**Files:**
- Modify: `src/platform/audio.h` (add factory)
- Create: `src/audio/factory.cpp`

- [ ] **Step 1: Update platform/audio.h with factory**

```cpp
// Append to src/platform/audio.h

// Factory function - tries PipeWire first, falls back to PulseAudio
AudioInput* createAudioInput();
void deleteAudioInput(AudioInput* input);
```

- [ ] **Step 2: Create audio/factory.cpp**

```cpp
// src/audio/factory.cpp
#include "../platform/audio.h"

#ifdef HAVE_PIPEWIRE
#include "pipewire_input.h"
#endif

#ifdef HAVE_PULSEAUDIO
#include "pulseaudio_input.h"
#endif

#include <iostream>

AudioInput* createAudioInput() {
#ifdef HAVE_PIPEWIRE
  std::cout << "Attempting to create PipeWire audio input...\n";
  auto* input = new PipeWireInput();
  return input;
#elif HAVE_PULSEAUDIO
  std::cout << "PipeWire not available, using PulseAudio...\n";
  auto* input = new PulseAudioInput();
  return input;
#else
  std::cerr << "No audio backend available (PipeWire and PulseAudio not found)\n";
  return nullptr;
#endif
}

void deleteAudioInput(AudioInput* input) {
  delete input;
}
```

- [ ] **Step 3: Update src/CMakeLists.txt to include factory**

Add `audio/factory.cpp` to `MILKDROP_SOURCES` list in `src/CMakeLists.txt`

- [ ] **Step 4: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/audio/factory.cpp src/platform/audio.h src/CMakeLists.txt
git commit -m "feat: add audio input factory with fallback

- Factory tries PipeWire first, falls back to PulseAudio
- Build-time feature detection via CMake flags
- Graceful error if no backend available
- Centralizes audio input instantiation

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

## Phase 2: Graphics & Windowing

### Task 8: Implement GLFW Window

**Files:**
- Create: `src/window/glfw_window.h`
- Create: `src/window/glfw_window.cpp`
- Create: `src/window/input.h`

- [ ] **Step 1: Create window/input.h**

```cpp
// src/window/input.h
#pragma once
#include "../platform/types.h"
#include <GLFW/glfw3.h>

class InputHandler {
public:
  static InputState getInputState(GLFWwindow* window);
  
private:
  static bool isKeyPressed(GLFWwindow* window, int glfwKey);
};
```

- [ ] **Step 2: Create window/glfw_window.h**

```cpp
// src/window/glfw_window.h
#pragma once
#include "../platform/window.h"
#include <GLFW/glfw3.h>

class GLFWWindow : public Window {
public:
  GLFWWindow();
  ~GLFWWindow();
  
  bool create(int width, int height, bool fullscreen) override;
  void shutdown() override;
  
  bool update() override;
  bool shouldClose() override;
  
  void getInputState(InputState& outState) override;
  void makeGLContextCurrent() override;
  void swapBuffers() override;
  
  int getWidth() const override { return width_; }
  int getHeight() const override { return height_; }
  
private:
  GLFWwindow* window_;
  int width_;
  int height_;
  bool isInitialized_;
  
  static void errorCallback(int error, const char* description);
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
```

- [ ] **Step 3: Create window/glfw_window.cpp**

```cpp
// src/window/glfw_window.cpp
#include "glfw_window.h"
#include "input.h"
#include <iostream>
#include <stdexcept>

// Global error callback
void glfwErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

GLFWWindow::GLFWWindow()
  : window_(nullptr), width_(0), height_(0), isInitialized_(false) {
  glfwSetErrorCallback(glfwErrorCallback);
}

GLFWWindow::~GLFWWindow() {
  shutdown();
}

bool GLFWWindow::create(int width, int height, bool fullscreen) {
  try {
    width_ = width;
    height_ = height;
    
    if (!glfwInit()) {
      std::cerr << "Failed to initialize GLFW\n";
      return false;
    }
    
    // Request OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    
    // Create window
    GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    window_ = glfwCreateWindow(width, height, "Milkdrop3", monitor, nullptr);
    
    if (!window_) {
      std::cerr << "Failed to create GLFW window\n";
      glfwTerminate();
      return false;
    }
    
    makeGLContextCurrent();
    
    // Enable V-Sync
    glfwSwapInterval(1);
    
    isInitialized_ = true;
    std::cout << "GLFW window created (" << width << "x" << height << ")\n";
    return true;
  } catch (const std::exception& e) {
    std::cerr << "GLFW initialization error: " << e.what() << "\n";
    return false;
  }
}

void GLFWWindow::shutdown() {
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  
  glfwTerminate();
  isInitialized_ = false;
}

bool GLFWWindow::update() {
  if (!isInitialized_) return false;
  glfwPollEvents();
  return true;
}

bool GLFWWindow::shouldClose() {
  if (!isInitialized_) return true;
  return glfwWindowShouldClose(window_);
}

void GLFWWindow::getInputState(InputState& outState) {
  outState = InputHandler::getInputState(window_);
}

void GLFWWindow::makeGLContextCurrent() {
  if (window_) {
    glfwMakeContextCurrent(window_);
  }
}

void GLFWWindow::swapBuffers() {
  if (window_) {
    glfwSwapBuffers(window_);
  }
}

// Input handler implementation
InputState InputHandler::getInputState(GLFWwindow* window) {
  InputState state = {};
  
  if (!window) return state;
  
  state.key_escape = isKeyPressed(window, GLFW_KEY_ESCAPE);
  state.key_p = isKeyPressed(window, GLFW_KEY_P);
  state.key_f = isKeyPressed(window, GLFW_KEY_F);
  state.key_s = isKeyPressed(window, GLFW_KEY_S);
  state.key_left = isKeyPressed(window, GLFW_KEY_LEFT);
  state.key_right = isKeyPressed(window, GLFW_KEY_RIGHT);
  state.key_up = isKeyPressed(window, GLFW_KEY_UP);
  state.key_down = isKeyPressed(window, GLFW_KEY_DOWN);
  
  return state;
}

bool InputHandler::isKeyPressed(GLFWwindow* window, int glfwKey) {
  return glfwGetKey(window, glfwKey) == GLFW_PRESS;
}
```

- [ ] **Step 4: Create window factory function**

Update `src/platform/window.h` to add:

```cpp
Window* createWindow();
void deleteWindow(Window* window);
```

Create `src/window/factory.cpp`:

```cpp
// src/window/factory.cpp
#include "../platform/window.h"
#include "glfw_window.h"

Window* createWindow() {
  return new GLFWWindow();
}

void deleteWindow(Window* window) {
  delete window;
}
```

- [ ] **Step 5: Update src/CMakeLists.txt**

Add to `MILKDROP_SOURCES`:
- `window/glfw_window.cpp`
- `window/factory.cpp`

Add `target_link_libraries(milkdrop3 glfw)`

- [ ] **Step 6: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/window/ src/platform/window.h src/CMakeLists.txt
git commit -m "feat: implement GLFW window with input handling

- OpenGL 3.3 core context creation
- Fullscreen and windowed modes
- Keyboard input mapping (preset nav, toggles)
- V-Sync support
- Input state polling
- Window factory for abstraction

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

### Task 9: Implement OpenGL Rendering Device

**Files:**
- Create: `src/graphics/opengl_device.h`
- Create: `src/graphics/opengl_device.cpp`
- Create: `src/graphics/shader.h`
- Create: `src/graphics/shader.cpp`

- [ ] **Step 1: Create graphics/shader.h**

```cpp
// src/graphics/shader.h
#pragma once
#include "../platform/graphics.h"
#include <string>
#include <glm/glm.hpp>

class OpenGLShader : public Shader {
public:
  OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
  ~OpenGLShader();
  
  uint32_t getHandle() const override { return programHandle_; }
  bool isValid() const override { return isValid_; }
  std::string getErrorLog() const override { return errorLog_; }
  
  void use() const;
  void setUniform(const std::string& name, float value) const;
  void setUniform(const std::string& name, const glm::vec3& value) const;
  void setUniform(const std::string& name, const glm::mat4& value) const;
  
private:
  uint32_t programHandle_;
  bool isValid_;
  std::string errorLog_;
  
  bool compile(const std::string& vertexSrc, const std::string& fragmentSrc);
  uint32_t compileShaderStage(const std::string& source, uint32_t stage);
};
```

- [ ] **Step 2: Create graphics/shader.cpp**

```cpp
// src/graphics/shader.cpp
#include "shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
  : programHandle_(0), isValid_(false) {
  compile(vertexSrc, fragmentSrc);
}

OpenGLShader::~OpenGLShader() {
  if (programHandle_) {
    glDeleteProgram(programHandle_);
  }
}

uint32_t OpenGLShader::compileShaderStage(const std::string& source, uint32_t stage) {
  uint32_t shader = glCreateShader(stage);
  const char* src = source.c_str();
  
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    errorLog_ = std::string(infoLog);
    std::cerr << "Shader compilation error:\n" << infoLog << "\n";
    glDeleteShader(shader);
    return 0;
  }
  
  return shader;
}

bool OpenGLShader::compile(const std::string& vertexSrc, const std::string& fragmentSrc) {
  uint32_t vertex = compileShaderStage(vertexSrc, GL_VERTEX_SHADER);
  if (!vertex) return false;
  
  uint32_t fragment = compileShaderStage(fragmentSrc, GL_FRAGMENT_SHADER);
  if (!fragment) {
    glDeleteShader(vertex);
    return false;
  }
  
  programHandle_ = glCreateProgram();
  glAttachShader(programHandle_, vertex);
  glAttachShader(programHandle_, fragment);
  glLinkProgram(programHandle_);
  
  int success;
  glGetProgramiv(programHandle_, GL_LINK_STATUS, &success);
  
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(programHandle_, 512, nullptr, infoLog);
    errorLog_ = std::string(infoLog);
    std::cerr << "Shader linking error:\n" << infoLog << "\n";
    return false;
  }
  
  isValid_ = true;
  return true;
}

void OpenGLShader::use() const {
  glUseProgram(programHandle_);
}

void OpenGLShader::setUniform(const std::string& name, float value) const {
  int loc = glGetUniformLocation(programHandle_, name.c_str());
  if (loc != -1) {
    glUniform1f(loc, value);
  }
}

void OpenGLShader::setUniform(const std::string& name, const glm::vec3& value) const {
  int loc = glGetUniformLocation(programHandle_, name.c_str());
  if (loc != -1) {
    glUniform3fv(loc, 1, glm::value_ptr(value));
  }
}

void OpenGLShader::setUniform(const std::string& name, const glm::mat4& value) const {
  int loc = glGetUniformLocation(programHandle_, name.c_str());
  if (loc != -1) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
  }
}
```

- [ ] **Step 3: Create graphics/opengl_device.h**

```cpp
// src/graphics/opengl_device.h
#pragma once
#include "../platform/graphics.h"
#include "../platform/window.h"
#include "shader.h"
#include <unordered_map>
#include <memory>

class OpenGLTexture : public Texture {
public:
  OpenGLTexture(int width, int height, const void* data);
  ~OpenGLTexture();
  
  uint32_t getHandle() const override { return textureHandle_; }
  int getWidth() const override { return width_; }
  int getHeight() const override { return height_; }
  
private:
  uint32_t textureHandle_;
  int width_;
  int height_;
};

class OpenGLDevice : public GraphicsDevice {
public:
  OpenGLDevice();
  ~OpenGLDevice();
  
  bool initialize(Window* window) override;
  void shutdown() override;
  
  Shader* createShader(const std::string& vertexSrc, const std::string& fragmentSrc) override;
  void deleteShader(Shader* shader) override;
  
  Texture* createTexture(int width, int height, const void* data) override;
  void deleteTexture(Texture* texture) override;
  
  void clear(float r, float g, float b, float a) override;
  void executeRenderCommand(const RenderCommand& cmd) override;
  void present() override;
  
private:
  Window* window_;
  bool isInitialized_;
  
  bool initializeGLAD();
};
```

- [ ] **Step 4: Create graphics/opengl_device.cpp**

```cpp
// src/graphics/opengl_device.cpp
#include "opengl_device.h"
#include <glad/glad.h>
#include <iostream>

// OpenGLTexture implementation
OpenGLTexture::OpenGLTexture(int width, int height, const void* data)
  : textureHandle_(0), width_(width), height_(height) {
  
  glGenTextures(1, &textureHandle_);
  glBindTexture(GL_TEXTURE_2D, textureHandle_);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  
  glBindTexture(GL_TEXTURE_2D, 0);
}

OpenGLTexture::~OpenGLTexture() {
  if (textureHandle_) {
    glDeleteTextures(1, &textureHandle_);
  }
}

// OpenGLDevice implementation
OpenGLDevice::OpenGLDevice()
  : window_(nullptr), isInitialized_(false) {
}

OpenGLDevice::~OpenGLDevice() {
  shutdown();
}

bool OpenGLDevice::initialize(Window* window) {
  try {
    window_ = window;
    
    if (!window_) {
      std::cerr << "Window required for graphics device initialization\n";
      return false;
    }
    
    window_->makeGLContextCurrent();
    
    // Initialize GLAD
    if (!initializeGLAD()) {
      std::cerr << "Failed to initialize GLAD\n";
      return false;
    }
    
    // Set OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glViewport(0, 0, window_->getWidth(), window_->getHeight());
    
    isInitialized_ = true;
    std::cout << "OpenGL device initialized\n";
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    return true;
  } catch (const std::exception& e) {
    std::cerr << "OpenGL initialization error: " << e.what() << "\n";
    return false;
  }
}

void OpenGLDevice::shutdown() {
  isInitialized_ = false;
}

bool OpenGLDevice::initializeGLAD() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to load OpenGL functions with GLAD\n";
    return false;
  }
  return true;
}

Shader* OpenGLDevice::createShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
  auto shader = new OpenGLShader(vertexSrc, fragmentSrc);
  
  if (!shader->isValid()) {
    std::cerr << "Shader creation failed: " << shader->getErrorLog() << "\n";
    delete shader;
    return nullptr;
  }
  
  return shader;
}

void OpenGLDevice::deleteShader(Shader* shader) {
  delete shader;
}

Texture* OpenGLDevice::createTexture(int width, int height, const void* data) {
  return new OpenGLTexture(width, height, data);
}

void OpenGLDevice::deleteTexture(Texture* texture) {
  delete texture;
}

void OpenGLDevice::clear(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLDevice::executeRenderCommand(const RenderCommand& cmd) {
  // TODO: Implement render command execution
  // This will draw geometry with the specified shader
}

void OpenGLDevice::present() {
  if (window_) {
    window_->swapBuffers();
  }
}
```

- [ ] **Step 5: Create graphics factory**

Create `src/graphics/factory.cpp`:

```cpp
// src/graphics/factory.cpp
#include "../platform/graphics.h"
#include "opengl_device.h"

GraphicsDevice* createGraphicsDevice() {
  return new OpenGLDevice();
}
```

- [ ] **Step 6: Update src/CMakeLists.txt**

Add to `MILKDROP_SOURCES`:
- `graphics/opengl_device.cpp`
- `graphics/shader.cpp`
- `graphics/factory.cpp`

Note: GLAD needs to be set up. For now, add a comment:
```cmake
# TODO: Add GLAD (OpenGL loader) - consider using:
# https://github.com/Dav1dde/glad
# or a system package if available
```

- [ ] **Step 7: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/graphics/ src/platform/graphics.h src/CMakeLists.txt
git commit -m "feat: implement OpenGL rendering device with shader support

- OpenGL 3.3 core context
- Shader compilation with GLSL
- Texture creation and management
- Uniform setting (float, vec3, mat4)
- Blending state configuration
- GLAD integration (TODO: full setup)
- Render command scaffold

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

## Phase 3: Integration & Main Loop

### Task 10: Implement Preset Manager

**Files:**
- Create: `src/ui/preset_manager.h`
- Create: `src/ui/preset_manager.cpp`

- [ ] **Step 1: Create ui/preset_manager.h**

```cpp
// src/ui/preset_manager.h
#pragma once
#include <string>
#include <vector>

struct PresetInfo {
  std::string filename;
  std::string name;
  std::string path;
};

class PresetManager {
public:
  PresetManager();
  
  // Scan and discover presets
  bool scanPresets();
  
  // Get available presets
  const std::vector<PresetInfo>& getPresets() const { return presets_; }
  
  // Load preset file
  std::string loadPreset(const std::string& presetPath);
  std::string loadPresetByIndex(int index);
  
  // Navigate
  int getCurrentPresetIndex() const { return currentIndex_; }
  void nextPreset();
  void previousPreset();
  
  // Get paths
  std::vector<std::string> getPresetSearchPaths() const;
  
private:
  std::vector<PresetInfo> presets_;
  int currentIndex_;
  
  void scanDirectory(const std::string& dir);
};
```

- [ ] **Step 2: Create ui/preset_manager.cpp**

```cpp
// src/ui/preset_manager.cpp
#include "preset_manager.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

PresetManager::PresetManager()
  : currentIndex_(0) {
}

std::vector<std::string> PresetManager::getPresetSearchPaths() const {
  std::vector<std::string> paths;
  
  // XDG user data directory
  const char* xdgData = std::getenv("XDG_DATA_HOME");
  if (xdgData && fs::path(xdgData).is_absolute()) {
    paths.push_back(std::string(xdgData) + "/milkdrop/presets");
  } else {
    const char* home = std::getenv("HOME");
    if (home) {
      paths.push_back(std::string(home) + "/.local/share/milkdrop/presets");
    }
  }
  
  // System-wide presets
  paths.push_back("/usr/share/milkdrop/presets");
  paths.push_back("/usr/local/share/milkdrop/presets");
  
  // Bundled presets relative to binary
  paths.push_back("./presets");
  paths.push_back("../share/presets");
  
  return paths;
}

void PresetManager::scanDirectory(const std::string& dir) {
  try {
    if (!fs::exists(dir)) {
      return;
    }
    
    for (const auto& entry : fs::directory_iterator(dir)) {
      if (entry.path().extension() == ".milk") {
        PresetInfo info;
        info.filename = entry.path().filename().string();
        info.name = entry.path().stem().string();
        info.path = entry.path().string();
        presets_.push_back(info);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error scanning preset directory " << dir << ": " << e.what() << "\n";
  }
}

bool PresetManager::scanPresets() {
  presets_.clear();
  currentIndex_ = 0;
  
  for (const auto& path : getPresetSearchPaths()) {
    scanDirectory(path);
  }
  
  if (presets_.empty()) {
    std::cout << "No presets found\n";
    return false;
  }
  
  std::cout << "Found " << presets_.size() << " presets\n";
  return true;
}

std::string PresetManager::loadPreset(const std::string& presetPath) {
  try {
    std::ifstream file(presetPath);
    if (!file.is_open()) {
      std::cerr << "Failed to open preset: " << presetPath << "\n";
      return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    std::cout << "Loaded preset: " << presetPath << "\n";
    return content;
  } catch (const std::exception& e) {
    std::cerr << "Error loading preset: " << e.what() << "\n";
    return "";
  }
}

std::string PresetManager::loadPresetByIndex(int index) {
  if (index < 0 || index >= (int)presets_.size()) {
    std::cerr << "Invalid preset index: " << index << "\n";
    return "";
  }
  
  currentIndex_ = index;
  return loadPreset(presets_[index].path);
}

void PresetManager::nextPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ + 1) % presets_.size();
}

void PresetManager::previousPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ - 1 + presets_.size()) % presets_.size();
}
```

- [ ] **Step 3: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/ui/preset_manager.*
git commit -m "feat: implement preset manager with XDG discovery

- Scan for .milk preset files in standard locations
- XDG_DATA_HOME and ~/.local/share/milkdrop support
- System-wide and bundled preset paths
- Load preset files by index or path
- Navigate through preset list
- Graceful handling of missing presets

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

### Task 11: Create Main Application Entry Point

**Files:**
- Create: `src/main.cpp`

- [ ] **Step 1: Create main.cpp**

```cpp
// src/main.cpp
#include <iostream>
#include <memory>

#include "platform/audio.h"
#include "platform/window.h"
#include "platform/graphics.h"
#include "platform/config.h"
#include "audio/audio_analyzer.h"
#include "ui/preset_manager.h"

class Milkdrop3Application {
public:
  bool initialize() {
    std::cout << "Initializing Milkdrop3...\n";
    
    // Load configuration
    auto& configMgr = ConfigManager::getInstance();
    if (!configMgr.load()) {
      std::cerr << "Warning: failed to load config, using defaults\n";
    }
    
    // Create window
    window_ = std::unique_ptr<Window>(createWindow());
    if (!window_) {
      std::cerr << "Failed to create window\n";
      return false;
    }
    
    auto& config = configMgr.getConfig();
    if (!window_->create(config.resolutionWidth, config.resolutionHeight, config.fullscreen)) {
      std::cerr << "Failed to initialize window\n";
      return false;
    }
    
    // Create graphics device
    graphics_ = std::unique_ptr<GraphicsDevice>(createGraphicsDevice());
    if (!graphics_) {
      std::cerr << "Failed to create graphics device\n";
      return false;
    }
    
    if (!graphics_->initialize(window_.get())) {
      std::cerr << "Failed to initialize graphics device\n";
      return false;
    }
    
    // Create audio input
    audio_ = std::unique_ptr<AudioInput>(createAudioInput());
    if (!audio_) {
      std::cerr << "Failed to create audio input\n";
      return false;
    }
    
    if (!audio_->initialize(config.audioDevice)) {
      std::cerr << "Failed to initialize audio input\n";
      return false;
    }
    
    // Initialize preset manager
    presetMgr_ = std::make_unique<PresetManager>();
    if (!presetMgr_->scanPresets()) {
      std::cerr << "Warning: no presets found\n";
    }
    
    // Load first preset or default
    if (!presetMgr_->getPresets().empty()) {
      currentPreset_ = presetMgr_->loadPresetByIndex(0);
      std::cout << "Loaded initial preset\n";
    }
    
    audioAnalyzer_ = std::make_unique<AudioAnalyzer>(512);
    
    isRunning_ = true;
    return true;
  }
  
  void shutdown() {
    std::cout << "Shutting down Milkdrop3...\n";
    isRunning_ = false;
    
    graphics_.reset();
    window_.reset();
    audio_.reset();
    
    auto& configMgr = ConfigManager::getInstance();
    configMgr.save();
  }
  
  void run() {
    if (!initialize()) {
      std::cerr << "Failed to initialize application\n";
      return;
    }
    
    std::cout << "Starting main loop...\n";
    
    while (isRunning_ && !window_->shouldClose()) {
      update();
      render();
    }
    
    shutdown();
    std::cout << "Application terminated normally\n";
  }
  
private:
  std::unique_ptr<Window> window_;
  std::unique_ptr<GraphicsDevice> graphics_;
  std::unique_ptr<AudioInput> audio_;
  std::unique_ptr<PresetManager> presetMgr_;
  std::unique_ptr<AudioAnalyzer> audioAnalyzer_;
  
  std::string currentPreset_;
  bool isRunning_ = false;
  
  void update() {
    window_->update();
    
    // Handle input
    InputState input;
    window_->getInputState(input);
    
    if (input.key_escape || input.key_q) {
      isRunning_ = false;
    }
    
    if (input.key_left) {
      presetMgr_->previousPreset();
      currentPreset_ = presetMgr_->loadPresetByIndex(presetMgr_->getCurrentPresetIndex());
    }
    
    if (input.key_right) {
      presetMgr_->nextPreset();
      currentPreset_ = presetMgr_->loadPresetByIndex(presetMgr_->getCurrentPresetIndex());
    }
    
    if (input.key_f) {
      // TODO: Toggle fullscreen
    }
    
    if (input.key_p) {
      // TODO: Pause/play
    }
    
    // Get audio frame
    AudioFrame audioFrame;
    if (audio_->getAudioFrame(audioFrame)) {
      auto freqBins = audioAnalyzer_->analyze(audioFrame.samples);
      // TODO: Pass frequency bins to visualization engine
    }
  }
  
  void render() {
    // Clear screen
    graphics_->clear(0.0f, 0.0f, 0.0f, 1.0f);
    
    // TODO: Execute visualization engine render commands
    // graphics_->executeRenderCommand(...);
    
    // Present
    graphics_->present();
  }
};

int main() {
  try {
    Milkdrop3Application app;
    app.run();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return 1;
  }
}
```

- [ ] **Step 2: Add forward declarations to factory headers**

Update `src/platform/audio.h`, `src/platform/window.h`, `src/platform/graphics.h` to ensure function declarations are visible.

- [ ] **Step 3: Update src/CMakeLists.txt main source**

Ensure `main.cpp` is in `MILKDROP_SOURCES`.

- [ ] **Step 4: Test compilation**

```bash
cd /home/drew/Documents/molkdroop/build
cmake ..
make
```

Expected: Compilation may succeed or have minor errors depending on GLAD setup. Document any issues for Task 12.

- [ ] **Step 5: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/main.cpp src/CMakeLists.txt
git commit -m "feat: create main application entry point and event loop

- Initialize all subsystems (window, graphics, audio, presets)
- Main event loop: input → update → render
- Preset navigation (arrow keys)
- Input handling for controls (F, P, Q)
- Audio frame capture and frequency analysis
- Configuration load/save integration

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

## Phase 3: Compilation & Dependency Resolution

### Task 12: Set Up GLAD and Resolve Build Issues

**Files:**
- Modify: `CMakeLists.txt`
- Create: `src/CMakeLists.txt` (update)

- [ ] **Step 1: Document GLAD setup strategy**

Two options for GLAD:
1. **Manual**: Download from https://glad.dav1d.de/ (generate loader for OpenGL 3.3 Core)
2. **Package Manager**: Check system for `libglad-dev` or similar

Choose option 1 (manual) for maximum portability.

- [ ] **Step 2: Add GLAD to CMakeLists.txt root**

```cmake
# Add after other find_package calls in root CMakeLists.txt

# GLAD loader (header-only version)
# Download from https://glad.dav1d.de/ if not present
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/include/glad/glad.h")
  message(WARNING "GLAD not found. Download from https://glad.dav1d.de/:
  1. Select OpenGL version 3.3 (Core profile)
  2. Extract to include/ directory
  Continuing without GLAD - build will fail at link time")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
```

- [ ] **Step 3: Create stub glad.c for linking**

Create `src/glad.c`:

```c
// src/glad.c
#define GLAD_GL_IMPLEMENTATION
#include "glad/glad.h"
```

Add `src/glad.c` to `MILKDROP_SOURCES` in `src/CMakeLists.txt`.

- [ ] **Step 4: Add GLFW include directory**

Update `src/CMakeLists.txt`:

```cmake
target_include_directories(milkdrop3 PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)
```

- [ ] **Step 5: Document build instructions**

Create `BUILD.md`:

```markdown
# Building Milkdrop3

## Prerequisites

- CMake 3.16+
- C++17 compiler (gcc, clang)
- GLFW3 dev libraries
- OpenGL dev libraries
- PipeWire or PulseAudio dev libraries
- GLM header-only library

## On Ubuntu/Debian:

\`\`\`bash
sudo apt-get install cmake g++ libglfw3-dev libgl1-mesa-dev \
  libpipewire-0.3-dev libpulse-dev libglm-dev
\`\`\`

## Setup GLAD

1. Download loader from https://glad.dav1d.de/
   - Select: OpenGL 3.3 Core
   - Language: C/C++
2. Extract to project root:
\`\`\`bash
unzip glad.zip -d .
cp -r glad/include . 
\`\`\`

## Build

\`\`\`bash
mkdir build && cd build
cmake ..
make
./milkdrop3
\`\`\`
```

- [ ] **Step 6: Commit**

```bash
cd /home/drew/Documents/molkdroop
git add src/glad.c CMakeLists.txt BUILD.md
git commit -m "build: add GLAD loader and build documentation

- GLAD setup with manual download from https://glad.dav1d.de/
- Stub glad.c for linking
- CMake configuration for include paths
- BUILD.md with prerequisites and build steps
- Ubuntu/Debian package list

Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>"
```

---

## Summary

This plan covers the core infrastructure for the Milkdrop3 Linux port. The next phases (preset compatibility, visualization engine integration, performance optimization) are substantial and should be planned after these foundation tasks are complete and the build is working.

**Key deliverables at this point:**
- ✅ CMake build system
- ✅ Platform abstraction interfaces
- ✅ Audio input (PipeWire/PulseAudio scaffolds)
- ✅ GLFW windowing with OpenGL rendering
- ✅ Preset manager with XDG support
- ✅ Main application loop
- ✅ Build documentation

**Next steps (Phase 4+):**
1. Integrate Milkdrop3 core visualization engine
2. Implement FFT with real library (FFTPACK, KissFFT, or Pffft)
3. Implement preset DSL parsing (from Milkdrop3 source)
4. Complete audio frame callbacks (PipeWire/PulseAudio)
5. Test with preset library
6. Performance profiling and optimization
7. UI polish (preset browser, overlays)

