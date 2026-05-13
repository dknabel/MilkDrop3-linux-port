# Phase 4.5: UI Refinement, Real Preset Testing, Audio Reliability, and Application Stability

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to execute this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the Milkdrop3 Linux port by implementing a functional UI, real preset testing, audio reliability improvements, and crash resilience to deliver a stable, production-ready visualization application.

**Architecture:** Build a complete application layer with GLFW windowing, OpenGL rendering loop, preset filesystem browser, visual feedback (FPS/frequency display), integrated error recovery for audio, and comprehensive crash resilience testing.

**Tech Stack:** GLFW3, OpenGL 3.3, filesystem library (C++17), unit and integration tests with Google Test, comprehensive exception handling.

---

## Context

Previous phases completed:
- Phase 4.1: KissFFT real-time frequency analysis (512-bin FFT)
- Phase 4.2: PipeWire/PulseAudio audio capture callbacks
- Phase 4.3: Milkdrop3 visualization core integration with EEL2 DSL interpreter
- Phase 4.4: EEL2 preset parser, expression evaluator, performance optimization

This phase closes the loop: UI, testing, reliability, and stability to reach production readiness.

---

## Phase 4.5 Tasks

### Task 1: Create Display Manager for Window and Rendering Loop

**Files:**
- Create: `src/ui/display_manager.h` - Window and render loop interface
- Create: `src/ui/display_manager.cpp` - GLFW integration and OpenGL rendering
- Create: `src/ui/CMakeLists.txt` - UI library target
- Modify: `src/CMakeLists.txt` - Add ui subdirectory

**Specification:**

Create `src/ui/display_manager.h`:

```cpp
#pragma once
#include <glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "../graphics/render_command.h"

class DisplayManager {
public:
  DisplayManager();
  ~DisplayManager();
  
  // Initialize GLFW window and OpenGL context
  bool initialize(int width = 1280, int height =720, const std::string& title = "Milkdrop3");
  
  // Process events and update window state
  bool update();
  
  // Render a frame from render commands
  void render(const std::vector<RenderCommand>& commands);
  
  // Cleanup and shutdown
  void shutdown();
  
  // Query state
  bool isRunning() const { return !glfwWindowShouldClose(window_); }
  glm::ivec2 getWindowSize() const { return windowSize_; }
  float getAspectRatio() const { return static_cast<float>(windowSize_.x) / windowSize_.y; }
  double getElapsedTime() const;
  
  // Input handling
  bool isKeyPressed(int key) const;
  
private:
  GLFWwindow* window_;
  glm::ivec2 windowSize_;
  double lastFrameTime_;
  
  void setupOpenGL();
  void handleInput();
};
```

Create `src/ui/display_manager.cpp`:

```cpp
#include "display_manager.h"
#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

static void glfwErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error (" << error << "): " << description << "\n";
}

DisplayManager::DisplayManager()
  : window_(nullptr), windowSize_(1280, 720), lastFrameTime_(0.0) {
}

DisplayManager::~DisplayManager() {
  shutdown();
}

bool DisplayManager::initialize(int width, int height, const std::string& title) {
  glfwSetErrorCallback(glfwErrorCallback);
  
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return false;
  }
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  
  window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!window_) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return false;
  }
  
  windowSize_ = glm::ivec2(width, height);
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1); // Enable vsync
  
  // Load OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to load OpenGL functions\n";
    glfwDestroyWindow(window_);
    glfwTerminate();
    return false;
  }
  
  setupOpenGL();
  lastFrameTime_ = glfwGetTime();
  
  std::cout << "Display manager initialized: " << width << "x" << height << "\n";
  return true;
}

void DisplayManager::setupOpenGL() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glViewport(0, 0, windowSize_.x, windowSize_.y);
  
  std::cout << "OpenGL " << glGetString(GL_VERSION) << " initialized\n";
}

bool DisplayManager::update() {
  if (!window_ || glfwWindowShouldClose(window_)) {
    return false;
  }
  
  glfwPollEvents();
  handleInput();
  
  int newWidth, newHeight;
  glfwGetWindowSize(window_, &newWidth, &newHeight);
  if (newWidth != windowSize_.x || newHeight != windowSize_.y) {
    windowSize_ = glm::ivec2(newWidth, newHeight);
    glViewport(0, 0, windowSize_.x, windowSize_.y);
  }
  
  return true;
}

void DisplayManager::render(const std::vector<RenderCommand>& commands) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Execute render commands (will be implemented with graphics device)
  // For now, this is a placeholder
  for (const auto& cmd : commands) {
    // Command execution deferred to graphics device
  }
  
  glfwSwapBuffers(window_);
}

void DisplayManager::shutdown() {
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  glfwTerminate();
}

double DisplayManager::getElapsedTime() const {
  return glfwGetTime();
}

bool DisplayManager::isKeyPressed(int key) const {
  if (!window_) return false;
  return glfwGetKey(window_, key) == GLFW_PRESS;
}

void DisplayManager::handleInput() {
  if (isKeyPressed(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window_, true);
  }
}
```

Create `src/ui/CMakeLists.txt`:

```cmake
# UI Components Library
add_library(ui STATIC
  display_manager.cpp
)

target_include_directories(ui PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${glfw3_SOURCE_DIR}/include
  ${glm_SOURCE_DIR}
)

target_link_libraries(ui
  glfw
  glad
  OpenGL::OpenGL
)
```

Modify `src/CMakeLists.txt` to add:

```cmake
add_subdirectory(ui)
```

And in the milkdrop3 target linking, add:

```cmake
target_link_libraries(milkdrop3 ui)
```

**Expected output:**
- DisplayManager class compiles and links
- Window initialization succeeds
- OpenGL context properly set up
- No memory leaks in initialization/shutdown
- All tests pass

---

### Task 2: Integrate Display Manager into Main Event Loop

**Files:**
- Modify: `src/main.cpp` - Wire DisplayManager into Milkdrop3Application

**Specification:**

Update `Milkdrop3Application` in `src/main.cpp`:

```cpp
#include "ui/display_manager.h"

class Milkdrop3Application {
private:
  std::unique_ptr<DisplayManager> display_;
  
public:
  bool initialize() override {
    // ... existing audio/visualization init ...
    
    // Initialize display
    display_ = std::make_unique<DisplayManager>();
    if (!display_->initialize(1280, 720, "Milkdrop3 - Linux")) {
      std::cerr << "Failed to initialize display manager\n";
      return false;
    }
    
    return true;
  }
  
  void run() override {
    if (!initialize()) {
      std::cerr << "Application initialization failed\n";
      return;
    }
    
    while (display_->isRunning()) {
      // Handle input
      if (display_->isKeyPressed(GLFW_KEY_P)) {
        // Play/pause
      }
      if (display_->isKeyPressed(GLFW_KEY_N)) {
        // Next preset
      }
      if (display_->isKeyPressed(GLFW_KEY_B)) {
        // Previous preset
      }
      
      // Update visualization
      auto audioFrame = audioInput_->getAudioFrame();
      if (!audioFrame.samples.empty()) {
        auto freqBins = audioAnalyzer_->analyze(audioFrame.samples);
        visualizer_->update(freqBins, 1.0f / 60.0f);
      }
      
      // Render
      auto commands = visualizer_->getRenderCommands();
      display_->render(commands);
      
      // Update display
      if (!display_->update()) {
        break;
      }
    }
    
    shutdown();
  }
  
  void shutdown() override {
    // ... existing cleanup ...
    display_.reset();
  }
};
```

Add keyboard control handlers with deltaTime tracking:

```cpp
float deltaTime = 0.0f;
double lastTime = display_->getElapsedTime();

while (display_->isRunning()) {
  double currentTime = display_->getElapsedTime();
  deltaTime = static_cast<float>(currentTime - lastTime);
  lastTime = currentTime;
  
  // Process audio and visualization with proper deltaTime
  // ...
}
```

**Expected output:**
- Application runs with window visible
- Input handling responsive
- No crashes during event loop
- Proper frame timing

---

### Task 3: Create Preset Browser for File Selection

**Files:**
- Create: `src/ui/preset_browser.h` - Filesystem preset discovery
- Create: `src/ui/preset_browser.cpp` - Preset listing and selection
- Modify: `src/ui/CMakeLists.txt` - Add preset_browser.cpp

**Specification:**

Create `src/ui/preset_browser.h`:

```cpp
#pragma once
#include <string>
#include <vector>
#include <filesystem>

class PresetBrowser {
public:
  PresetBrowser();
  
  // Scan preset directories and build list
  bool scanPresets(const std::string& defaultPath = "~/.milkdrop3/presets");
  
  // Get current preset list
  const std::vector<std::string>& getPresets() const { return presets_; }
  
  // Get current selection index
  int getCurrentIndex() const { return currentIndex_; }
  
  // Navigate presets
  void nextPreset();
  void previousPreset();
  
  // Get full path to current preset
  std::string getCurrentPresetPath() const;
  std::string getCurrentPresetName() const;
  
private:
  std::vector<std::string> presets_;
  std::string basePath_;
  int currentIndex_;
  
  bool loadPresetsFromDirectory(const std::filesystem::path& dir);
};
```

Create `src/ui/preset_browser.cpp`:

```cpp
#include "preset_browser.h"
#include <algorithm>
#include <iostream>
#include <cstdlib>

PresetBrowser::PresetBrowser() : currentIndex_(0) {}

bool PresetBrowser::scanPresets(const std::string& defaultPath) {
  // Expand ~ to home directory
  std::string expandedPath = defaultPath;
  if (expandedPath[0] == '~') {
    const char* home = std::getenv("HOME");
    if (home) {
      expandedPath = std::string(home) + expandedPath.substr(1);
    }
  }
  
  basePath_ = expandedPath;
  std::filesystem::path presetDir(basePath_);
  
  if (!std::filesystem::exists(presetDir)) {
    std::cout << "Preset directory not found: " << basePath_ << "\n";
    return false;
  }
  
  presets_.clear();
  return loadPresetsFromDirectory(presetDir);
}

bool PresetBrowser::loadPresetsFromDirectory(const std::filesystem::path& dir) {
  try {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
      if (entry.path().extension() == ".milk") {
        presets_.push_back(entry.path().filename().string());
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error scanning preset directory: " << e.what() << "\n";
    return false;
  }
  
  std::sort(presets_.begin(), presets_.end());
  
  if (!presets_.empty()) {
    std::cout << "Loaded " << presets_.size() << " presets\n";
    return true;
  }
  
  return false;
}

void PresetBrowser::nextPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ + 1) % presets_.size();
}

void PresetBrowser::previousPreset() {
  if (presets_.empty()) return;
  currentIndex_ = (currentIndex_ - 1 + presets_.size()) % presets_.size();
}

std::string PresetBrowser::getCurrentPresetPath() const {
  if (currentIndex_ < presets_.size()) {
    return basePath_ + "/" + presets_[currentIndex_];
  }
  return "";
}

std::string PresetBrowser::getCurrentPresetName() const {
  if (currentIndex_ < presets_.size()) {
    return presets_[currentIndex_];
  }
  return "";
}
```

Update `src/ui/CMakeLists.txt` to include `preset_browser.cpp`.

**Expected output:**
- Preset browser scans filesystem correctly
- Presets sorted alphabetically
- Navigation works (next/previous)
- Handles empty directories gracefully

---

### Task 4: Create Visual Feedback Display

**Files:**
- Create: `src/ui/visual_feedback.h` - FPS and frequency display
- Create: `src/ui/visual_feedback.cpp` - Text rendering and metrics display
- Modify: `src/ui/CMakeLists.txt` - Add visual_feedback.cpp

**Specification:**

Create `src/ui/visual_feedback.h`:

```cpp
#pragma once
#include <string>
#include <vector>
#include <deque>

class VisualFeedback {
public:
  VisualFeedback();
  
  // Update with frame time and frequency data
  void update(float deltaTime, const std::vector<float>& frequencyBins);
  
  // Get formatted display strings
  std::string getFpsDisplay() const;
  std::string getPresetInfo(const std::string& presetName) const;
  std::string getFrequencyDisplay() const;
  
  // Get average FPS over last N frames
  float getAverageFps() const;
  
private:
  std::deque<float> frameTimesMs_;
  static constexpr int MAX_FRAME_HISTORY = 60;
  
  float peakFrequency_;
  float averageFrequency_;
  int frequencyPeakBin_;
  
  void updateMetrics();
};
```

Create `src/ui/visual_feedback.cpp`:

```cpp
#include "visual_feedback.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <cmath>

VisualFeedback::VisualFeedback()
  : peakFrequency_(0.0f), averageFrequency_(0.0f), frequencyPeakBin_(0) {
}

void VisualFeedback::update(float deltaTime, const std::vector<float>& frequencyBins) {
  frameTimesMs_.push_back(deltaTime * 1000.0f);
  if (frameTimesMs_.size() > MAX_FRAME_HISTORY) {
    frameTimesMs_.pop_front();
  }
  
  // Find peak frequency
  if (!frequencyBins.empty()) {
    auto maxIt = std::max_element(frequencyBins.begin(), frequencyBins.end());
    peakFrequency_ = *maxIt;
    frequencyPeakBin_ = std::distance(frequencyBins.begin(), maxIt);
    averageFrequency_ = std::accumulate(frequencyBins.begin(), frequencyBins.end(), 0.0f) / frequencyBins.size();
  }
}

std::string VisualFeedback::getFpsDisplay() const {
  float fps = getAverageFps();
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1) << "FPS: " << fps;
  return oss.str();
}

std::string VisualFeedback::getPresetInfo(const std::string& presetName) const {
  std::ostringstream oss;
  oss << "Preset: " << presetName;
  return oss.str();
}

std::string VisualFeedback::getFrequencyDisplay() const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2)
      << "Peak: " << peakFrequency_ << " Avg: " << averageFrequency_;
  return oss.str();
}

float VisualFeedback::getAverageFps() const {
  if (frameTimesMs_.empty()) return 0.0f;
  
  float avgMs = std::accumulate(frameTimesMs_.begin(), frameTimesMs_.end(), 0.0f) / frameTimesMs_.size();
  if (avgMs <= 0.0f) return 0.0f;
  return 1000.0f / avgMs;
}
```

Update `src/ui/CMakeLists.txt` to include `visual_feedback.cpp`.

**Expected output:**
- FPS calculation accurate
- Frequency metrics computed correctly
- Display strings formatted properly
- No memory issues in metric tracking

---

### Task 5: Create Real Preset Integration Tests

**Files:**
- Create: `tests/integration/test_real_presets.cpp` - Load actual .milk presets
- Modify: `tests/CMakeLists.txt` - Add real preset test target

**Specification:**

Create `tests/integration/test_real_presets.cpp`:

```cpp
#include <gtest/gtest.h>
#include "core/preset/preset_parser.h"
#include "core/expression/expression_evaluator.h"
#include "core/visualization/visualization_engine.h"
#include <filesystem>
#include <vector>

class RealPresetTest : public ::testing::Test {
protected:
  PresetParser parser_;
  ExpressionEvaluator evaluator_;
  VisualizationEngine visualizer_;
  
  std::vector<std::string> findTestPresets() {
    std::vector<std::string> presets;
    std::filesystem::path testPresetsDir("tests/fixtures/presets");
    
    if (std::filesystem::exists(testPresetsDir)) {
      for (const auto& entry : std::filesystem::directory_iterator(testPresetsDir)) {
        if (entry.path().extension() == ".milk") {
          presets.push_back(entry.path().string());
        }
      }
    }
    
    return presets;
  }
};

TEST_F(RealPresetTest, ParseValidPresets) {
  auto presets = findTestPresets();
  EXPECT_GT(presets.size(), 0) << "No test presets found";
  
  for (const auto& presetPath : presets) {
    std::ifstream file(presetPath);
    EXPECT_TRUE(file.is_open()) << "Cannot open preset: " << presetPath;
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    bool parseSuccess = parser_.parse(content);
    EXPECT_TRUE(parseSuccess) << "Failed to parse: " << presetPath;
  }
}

TEST_F(RealPresetTest, EvaluatePerFrameExpressions) {
  auto presets = findTestPresets();
  if (presets.empty()) GTEST_SKIP_("No test presets available");
  
  for (const auto& presetPath : presets) {
    std::ifstream file(presetPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    EXPECT_TRUE(parser_.parse(content)) << "Parse failed: " << presetPath;
    
    // Try to evaluate per-frame equations
    std::vector<float> dummyFreq(256, 0.5f);
    EXPECT_NO_THROW({
      visualizer_.update(dummyFreq, 0.016f);
    }) << "Exception during evaluation for: " << presetPath;
  }
}

TEST_F(RealPresetTest, HandlesCorruptedPresets) {
  // Test parsing invalid .milk files
  std::string invalidPreset = "[invalid]\ngarbled data!@#$%\n";
  
  // Parser should not crash
  EXPECT_NO_THROW({
    parser_.parse(invalidPreset);
  });
}

TEST_F(RealPresetTest, MultiFrameExecution) {
  auto presets = findTestPresets();
  if (presets.empty()) GTEST_SKIP_("No test presets available");
  
  std::ifstream file(presets[0]);
  std::string content((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
  
  EXPECT_TRUE(parser_.parse(content));
  
  // Run 300 frames of evaluation
  std::vector<float> dummyFreq(256, 0.3f);
  for (int i = 0; i < 300; ++i) {
    EXPECT_NO_THROW({
      visualizer_.update(dummyFreq, 0.016f);
    }) << "Crash on frame " << i;
  }
}

TEST_F(RealPresetTest, SequentialPresetLoading) {
  auto presets = findTestPresets();
  if (presets.size() < 2) GTEST_SKIP_("Need at least 2 presets");
  
  std::vector<float> dummyFreq(256, 0.5f);
  
  // Load first preset, run 100 frames
  {
    std::ifstream file(presets[0]);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    EXPECT_TRUE(parser_.parse(content));
    
    for (int i = 0; i < 100; ++i) {
      visualizer_.update(dummyFreq, 0.016f);
    }
  }
  
  // Load second preset, run 100 frames
  {
    std::ifstream file(presets[1]);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    EXPECT_TRUE(parser_.parse(content));
    
    for (int i = 0; i < 100; ++i) {
      visualizer_.update(dummyFreq, 0.016f);
    }
  }
}
```

**Expected output:**
- All real preset tests pass
- Proper handling of valid presets
- Graceful failure on corrupted presets
- Multi-frame execution stable

---

### Task 6: Add Audio Error Recovery and Resilience

**Files:**
- Modify: `src/audio/pipewire_input.cpp` - Add reconnection logic
- Modify: `src/audio/pulseaudio_input.cpp` - Add reconnection logic
- Modify: `src/audio/audio_input.h` - Add reconnection interface

**Specification:**

Add to `src/audio/audio_input.h`:

```cpp
class AudioInput {
public:
  virtual ~AudioInput() = default;
  virtual bool initialize(const std::string& deviceName = "default") = 0;
  virtual AudioFrame getAudioFrame() = 0;
  virtual void shutdown() = 0;
  
  // New: Resilience interface
  virtual bool isConnected() const = 0;
  virtual bool attemptReconnect() = 0;
  virtual int getReconnectAttempts() const = 0;
};
```

Modify `src/audio/pipewire_input.cpp`:

```cpp
// Add member variables
int reconnectAttempts_ = 0;
static constexpr int MAX_RECONNECT_ATTEMPTS = 5;
bool isConnected_ = false;

bool PipeWireInput::isConnected() const {
  return isConnected_ && stream_;
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

int PipeWireInput::getReconnectAttempts() const {
  return reconnectAttempts_;
}

// In onStreamStateChange, set isConnected_ state
void PipeWireInput::onStreamStateChange(void* userData, pw_stream_state old,
                                        pw_stream_state state, const char* error) {
  PipeWireInput* self = reinterpret_cast<PipeWireInput*>(userData);
  
  if (state == PW_STREAM_STATE_STREAMING) {
    self->isConnected_ = true;
    std::cout << "PipeWire stream connected\n";
  } else if (state == PW_STREAM_STATE_ERROR || state == PW_STREAM_STATE_PAUSED) {
    self->isConnected_ = false;
    std::cerr << "PipeWire stream error/paused\n";
  }
}
```

Similar implementation for `src/audio/pulseaudio_input.cpp`.

**Expected output:**
- Reconnection logic compiles
- Graceful handling of disconnections
- Proper state tracking
- No crashes on audio device failure

---

### Task 7: Add Crash Resilience and Error Handling

**Files:**
- Create: `tests/stability/test_crash_resilience.cpp` - Comprehensive exception handling
- Modify: `src/main.cpp` - Add try/catch and error recovery

**Specification:**

Create `tests/stability/test_crash_resilience.cpp`:

```cpp
#include <gtest/gtest.h>
#include "ui/display_manager.h"
#include "audio/audio_input.h"
#include "core/visualization/visualization_engine.h"
#include <stdexcept>

class CrashResilienceTest : public ::testing::Test {
protected:
  // Test that exception in audio doesn't crash app
  void testAudioExceptionHandling() {
    // Simulate audio failure
    try {
      throw std::runtime_error("Audio device disconnected");
    } catch (const std::exception& e) {
      EXPECT_STREQ(e.what(), "Audio device disconnected");
    }
  }
};

TEST_F(CrashResilienceTest, AudioExceptionHandling) {
  EXPECT_NO_THROW({
    testAudioExceptionHandling();
  });
}

TEST_F(CrashResilienceTest, NullPointerHandling) {
  DisplayManager display;
  
  // Attempting operations on uninitialized display should not crash
  EXPECT_FALSE(display.isRunning());
  EXPECT_NO_THROW({
    display.update();
  });
}

TEST_F(CrashResilienceTest, InvalidPresetsDoNotCrash) {
  VisualizationEngine visualizer;
  
  // Invalid preset content
  std::string invalidPreset = "";
  EXPECT_FALSE(visualizer.loadPreset(invalidPreset));
  
  // Should still be operable
  std::vector<float> dummyFreq(256, 0.5f);
  EXPECT_NO_THROW({
    visualizer.update(dummyFreq, 0.016f);
  });
}

TEST_F(CrashResilienceTest, FreqBinVectorEdgeCases) {
  VisualizationEngine visualizer;
  
  // Empty frequency data
  std::vector<float> emptyFreq;
  EXPECT_NO_THROW({
    visualizer.update(emptyFreq, 0.016f);
  });
  
  // Single frequency bin
  std::vector<float> singleFreq = {0.5f};
  EXPECT_NO_THROW({
    visualizer.update(singleFreq, 0.016f);
  });
  
  // Very large frequency data
  std::vector<float> largeFreq(100000, 0.5f);
  EXPECT_NO_THROW({
    visualizer.update(largeFreq, 0.016f);
  });
}

TEST_F(CrashResilienceTest, RapidPresetSwitching) {
  VisualizationEngine visualizer;
  std::vector<float> dummyFreq(256, 0.5f);
  
  // Rapidly load and execute (simulating preset switching)
  for (int i = 0; i < 10; ++i) {
    visualizer.reset();
    visualizer.update(dummyFreq, 0.016f);
  }
}
```

Modify `src/main.cpp` to add comprehensive error handling:

```cpp
int main() {
  try {
    auto app = std::make_unique<Milkdrop3Application>();
    
    if (!app->initialize()) {
      std::cerr << "Failed to initialize application\n";
      return 1;
    }
    
    app->run();
    app->shutdown();
    
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "Unknown fatal error\n";
    return 1;
  }
  
  return 0;
}
```

**Expected output:**
- All resilience tests pass
- Graceful error handling in main loop
- No uncaught exceptions escape
- Proper cleanup on errors

---

### Task 8: End-to-End Application Test

**Files:**
- Create: `tests/integration/test_application.cpp` - Full pipeline test
- Modify: `tests/CMakeLists.txt` - Add application test target

**Specification:**

Create `tests/integration/test_application.cpp`:

```cpp
#include <gtest/gtest.h>
#include "ui/display_manager.h"
#include "ui/preset_browser.h"
#include "ui/visual_feedback.h"
#include "audio/audio_input.h"
#include "core/audio_analyzer.h"
#include "core/visualization/visualization_engine.h"
#include <thread>
#include <chrono>

class ApplicationTest : public ::testing::Test {
protected:
  std::unique_ptr<DisplayManager> display_;
  std::unique_ptr<PresetBrowser> browser_;
  std::unique_ptr<VisualFeedback> feedback_;
  std::unique_ptr<AudioAnalyzer> analyzer_;
  std::unique_ptr<VisualizationEngine> visualizer_;
  
  void SetUp() override {
    display_ = std::make_unique<DisplayManager>();
    browser_ = std::make_unique<PresetBrowser>();
    feedback_ = std::make_unique<VisualFeedback>();
    analyzer_ = std::make_unique<AudioAnalyzer>(512);
    visualizer_ = std::make_unique<VisualizationEngine>();
  }
};

TEST_F(ApplicationTest, ApplicationInitialization) {
  // Initialize all components
  EXPECT_FALSE(display_->isRunning()); // Not initialized yet
  
  // Window init would require X11 display, so skip in CI
  // EXPECT_TRUE(display_->initialize());
}

TEST_F(ApplicationTest, PresetBrowserIntegration) {
  // Scan presets (empty dir is OK)
  bool scanned = browser_->scanPresets("/tmp");
  // Should not crash even if dir is empty
  EXPECT_TRUE(scanned || browser_->getPresets().empty());
}

TEST_F(ApplicationTest, VisualizationPipeline) {
  // Simulate audio processing
  std::vector<float> audioSamples(512, 0.1f);
  auto freqBins = analyzer_->analyze(audioSamples);
  
  EXPECT_GT(freqBins.size(), 0);
  
  // Feed to visualizer
  EXPECT_NO_THROW({
    visualizer_->update(freqBins, 0.016f);
  });
  
  auto commands = visualizer_->getRenderCommands();
  EXPECT_FALSE(commands.empty()); // Should generate render commands
}

TEST_F(ApplicationTest, FullPipelineStability) {
  std::vector<float> audioSamples(512);
  
  // Simulate 5 seconds of continuous operation (300 frames @ 60 FPS)
  for (int frame = 0; frame < 300; ++frame) {
    // Generate dummy audio
    for (auto& sample : audioSamples) {
      sample = 0.1f * std::sin(2.0f * 3.14159f * frame / 60.0f);
    }
    
    // Analyze
    auto freqBins = analyzer_->analyze(audioSamples);
    
    // Update visualization
    EXPECT_NO_THROW({
      visualizer_->update(freqBins, 0.016f);
    });
    
    // Update feedback
    feedback_->update(0.016f, freqBins);
    
    // Check metrics
    float fps = feedback_->getAverageFps();
    EXPECT_GT(fps, 0.0f) << "FPS should be positive at frame " << frame;
  }
}

TEST_F(ApplicationTest, ComponentLifecycles) {
  // Create and destroy components multiple times
  for (int i = 0; i < 5; ++i) {
    auto display = std::make_unique<DisplayManager>();
    auto browser = std::make_unique<PresetBrowser>();
    auto feedback = std::make_unique<VisualFeedback>();
    // Components destroyed at end of scope
  }
  // Should not crash or leak memory
}
```

**Expected output:**
- All E2E tests pass
- Full pipeline stable for extended operation
- Component lifecycles manage properly
- No memory leaks detected

---

## Success Criteria

- [ ] DisplayManager window and rendering loop working
- [ ] Input handling responsive (P/F/N/Q keys)
- [ ] Preset browser scans filesystem correctly
- [ ] Visual feedback displays FPS and frequency metrics
- [ ] Real preset tests load actual .milk files
- [ ] Audio resilience handles disconnections
- [ ] Crash resilience tests pass
- [ ] Full E2E test runs 300 frames without crashing
- [ ] Application stable and feature-complete
- [ ] All tests passing

---

## Notes

- Phase 4.5 is the final phase before production deployment
- Focus on stability and user experience
- All error conditions should be handled gracefully
- Performance target: maintain 60 FPS with visual feedback
- Test coverage critical: integration, stability, and E2E tests
