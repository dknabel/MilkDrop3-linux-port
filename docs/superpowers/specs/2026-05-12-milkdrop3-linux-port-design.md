# Milkdrop3 Linux Port Design

**Date:** 2026-05-12  
**Project:** Milkdrop3 Linux Port  
**Goal:** Port Milkdrop3 (Windows audio visualizer) to Linux while maintaining 100% preset compatibility and feature parity with the core visualization engine.

---

## Overview

Milkdrop3 is a sophisticated audio-reactive visualizer with a custom DSL for animations and effects. The Windows version is written in C++/C and uses Windows-specific APIs for audio capture, windowing, and rendering. This project ports the core functionality to Linux by abstracting platform-specific layers and replacing them with Linux equivalents.

**Key Constraint:** System audio capture (visualize what's currently playing, e.g., Spotify, YouTube) rather than file loading.

---

## Architecture

### Core Components

**1. Audio Input Layer**

**Responsibility:** Capture real-time audio from the system and extract frequency data.

**Current (Windows):** WASAPI or DirectSound device API  
**Linux Implementation:**
- Primary: PipeWire (modern, flexible, becoming standard on Linux)
- Fallback: PulseAudio (wider compatibility on older systems)
- Runtime detection: try PipeWire first, fall back to PulseAudio

**User Setup:**
- On first run, detect available audio sources (microphone, system audio monitor, line-in)
- Let user select preferred source
- Remember selection in `~/.config/milkdrop/config.json`

**Interface:**
```cpp
class AudioInput {
  virtual bool initialize(const std::string& device) = 0;
  virtual void getAudioFrame(float* samples, int count) = 0;
  virtual std::vector<std::string> listDevices() = 0;
};

class PipeWireInput : public AudioInput { /* implementation */ };
class PulseAudioInput : public AudioInput { /* implementation */ };
```

**Frequency Analysis:** FFT (Fast Fourier Transform) converts raw samples to frequency bins. This logic is unchanged from Milkdrop3; only the audio source changes.

---

**2. Windowing & Input Layer**

**Responsibility:** Create window, handle keyboard/mouse input, manage event loop.

**Current (Windows):** Win32 API  
**Linux Implementation:** GLFW3 or SDL2 (both cross-platform, well-maintained)

**Recommendation:** GLFW3 (simpler, lighter-weight, good OpenGL support)

**Features:**
- Create fullscreen or windowed OpenGL context
- Keyboard input (preset navigation, parameter tweaks)
- Mouse input (UI interaction if needed)
- V-Sync and frame rate control

**Window Modes:**
- Fullscreen (immersive visualization)
- Windowed (for development/debugging)

**Interface:**
```cpp
class Window {
  virtual bool create(int width, int height, bool fullscreen) = 0;
  virtual void update() = 0;
  virtual bool shouldClose() = 0;
  virtual void getInputState(InputState& out) = 0;
};
```

---

**3. Graphics & Rendering Layer**

**Responsibility:** Device creation, resource management, rendering.

**Current (Windows):** Direct3D (DX11 or DX12)  
**Linux Implementation:** OpenGL 3.3+ (widely supported, stable)

**Why OpenGL:**
- Standard on Linux
- Good shader support (GLSL)
- Proven for this use case (Milkdrop history)
- Future upgrade path: Vulkan if performance is needed

**Shader Conversion:**
- Milkdrop presets use shader code (originally HLSL for Direct3D)
- Convert HLSL → GLSL: mostly mechanical (intrinsics map well)
- Example: `float4 color;` → `vec4 color;`, `tex2D()` → `texture()`
- Some Direct3D-isms need adaptation, but straightforward

**Interface:**
```cpp
class GraphicsDevice {
  virtual bool initialize(Window* window) = 0;
  virtual Shader* createShader(const std::string& glsl) = 0;
  virtual Texture* createTexture(int w, int h, void* data) = 0;
  virtual void render(const RenderCommand& cmd) = 0;
};

class OpenGLDevice : public GraphicsDevice { /* implementation */ };
```

---

**4. Core Visualization Engine (Unchanged)**

**Responsibility:** Parse presets, interpret DSL, compute animations, orchestrate rendering.

**Status:** Keep as-is from Milkdrop3 source. This is the value of porting.

**What It Does:**
- Load `.milk` preset files (text-based DSL)
- Interpret equations and animations
- Map frequency data to visual parameters
- Command rendering primitives (geometry, shaders, blending)

**Integration:** Feed this engine:
- Frequency bins (from audio layer)
- Time/timing info
- User input (preset changes, parameter tweaks)

---

**5. File System & Preset Management**

**Responsibility:** Store/load presets, configuration, handle platform paths.

**Preset Locations (Linux conventions):**
- User presets: `~/.local/share/milkdrop/presets/`
- Configuration: `~/.config/milkdrop/config.json`
- Bundled defaults: `/usr/share/milkdrop/presets/` or relative to binary

**Config Storage (JSON):**
```json
{
  "lastPreset": "Amon Tobin - Journeys.milk",
  "audioDevice": "default_monitor",
  "resolution": [1920, 1080],
  "fullscreen": true,
  "audioSensitivity": 1.0
}
```

**Preset Discovery:**
- Scan standard XDG directories on startup
- Display available presets in UI
- Load presets unchanged (DSL interpreter handles them)

**Compatibility:**
- Milkdrop3 presets: 100% compatible
- projectM presets: compatible (noted in Milkdrop3 source)
- User-created presets: work as-is

---

**6. UI & Control**

**Responsibility:** Present presets, allow user interaction, display settings.

**Minimal UI (for MVP):**
- Fullscreen visualization (primary)
- Keyboard shortcuts:
  - Arrow keys: cycle through presets
  - 'F': toggle fullscreen
  - 'P': pause/play
  - 'S': show preset list (overlay)
  - 'Q': quit

**Optional (Phase 2):**
- In-game preset browser with search
- Parameter sliders for presets that expose them
- Audio device selector UI

---

## Build System

**Build Tool:** CMake 3.16+

**Dependencies:**

| Library | Purpose | Version |
|---------|---------|---------|
| GLFW3 | Windowing & input | 3.3+ |
| OpenGL | Rendering | 3.3+ |
| PipeWire | Audio capture (primary) | 0.3.40+ |
| PulseAudio | Audio capture (fallback) | 13.0+ |
| GLM | Math library | 0.9.9+ |

**Project Structure:**
```
milkdrop3-linux/
├── src/
│   ├── core/           (visualization engine from Milkdrop3)
│   ├── platform/       (abstraction interfaces)
│   ├── audio/          (PipeWire/PulseAudio implementations)
│   ├── graphics/       (OpenGL implementation)
│   ├── ui/             (UI/controls)
│   ├── main.cpp        (entry point)
│   └── CMakeLists.txt
├── presets/            (bundled default presets)
├── docs/
│   └── superpowers/
│       └── specs/      (this design)
├── CMakeLists.txt
└── README.md
```

**CMakeLists.txt (top-level):**
```cmake
cmake_minimum_required(VERSION 3.16)
project(Milkdrop3 CXX C)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE libpipewire-0.3)
pkg_check_modules(PULSEAUDIO libpulse)

# Build targets
add_executable(milkdrop3 src/main.cpp [...])
target_link_libraries(milkdrop3
  OpenGL::OpenGL
  glfw
  ${PIPEWIRE_LIBRARIES}
  ${PULSEAUDIO_LIBRARIES}
)
```

**Compile on Linux:**
```bash
mkdir build && cd build
cmake ..
make
./milkdrop3
```

---

## Data Flow

1. **Startup:**
   - Initialize audio input (PipeWire/PulseAudio)
   - Create OpenGL window and context
   - Load configuration and default preset

2. **Main Loop (60 FPS):**
   - Read audio frame (raw samples) from audio input
   - Compute FFT → frequency bins
   - Pass frequency bins to visualization engine
   - Engine updates animations, outputs render commands
   - Render commands executed via OpenGL
   - Present frame to window
   - Handle input (preset changes, etc.)

3. **Preset Change:**
   - Load `.milk` file
   - Parse and validate DSL
   - Visualization engine resets animation state
   - Next frame renders with new preset

---

## Testing & Validation

**Manual Testing (primary):**
- Boot visualizer, confirm window opens
- Confirm audio input detected and working
- Load 5-10 presets from Milkdrop3 library
- Verify audio-reactive behavior (frequencies drive visualization)
- Test with multiple audio sources: Spotify, YouTube, local player
- Measure performance: target 60 FPS, reasonable CPU/GPU load

**Compatibility Checks:**
- Milkdrop3 preset library: all load and execute correctly
- projectM presets: subset work correctly
- Custom presets: accept user-created `.milk` files

**Performance Targets:**
- Target: 60 FPS at 1920x1080
- Should run on modest hardware (laptops, older desktops)
- GPU: any OpenGL 3.3+ capable card

**Known Limitations (acceptable for MVP):**
- Winamp integration: Windows-only, skip for Linux
- Some advanced Direct3D features: may not have perfect equivalents, graceful fallback
- Specific GPU drivers: may have quirks; document workarounds if found

---

## Implementation Phases

**Phase 1: Setup & Audio (Week 1)**
- Set up CMake build system
- Implement audio input layer (PipeWire/PulseAudio)
- Get audio capture working, verify frequency analysis

**Phase 2: Graphics & Window (Week 2)**
- Implement GLFW window and input
- Implement OpenGL rendering layer
- Convert sample shaders from HLSL → GLSL
- Confirm basic rendering works

**Phase 3: Integration (Week 3)**
- Integrate core visualization engine
- Wire up audio → engine → rendering
- Load and run first preset
- Debug compatibility issues

**Phase 4: Polish & Testing (Weeks 4-5)**
- Test with full preset library
- Performance optimization if needed
- UI refinement (preset browser, etc.)
- Documentation and packaging

**Timeline:** 3-5 weeks for functional port

---

## Success Criteria

- [ ] Code builds cleanly on Ubuntu 22.04 (or similar recent distro)
- [ ] Audio capture from system audio works
- [ ] Can load and run Milkdrop3 presets
- [ ] Presets respond correctly to audio frequencies
- [ ] Runs at 60 FPS on reasonable hardware (laptops acceptable)
- [ ] Preset library (50+ presets) tested and working
- [ ] Graceful error messages if audio device unavailable

---

## Risks & Mitigation

| Risk | Mitigation |
|------|-----------|
| Audio API fragmentation (PipeWire vs PulseAudio) | Implement both, runtime detection, clear docs on setup |
| HLSL→GLSL shader conversion bugs | Start with simple presets, incrementally test complex ones |
| Performance regression on older GPUs | Profile early, target OpenGL 3.3 (widely supported) |
| Missing dependencies on user's system | Document all dependencies, provide CMake fallbacks |
| Preset incompatibilities | Test library incrementally, document any known issues |

---

## Future Enhancements (Post-MVP)

- Vulkan rendering backend (better performance)
- Wayland support (newer Linux systems)
- Preset editor/customization UI
- Network/streaming support (share visualizations)
- Plugin system for custom effects

