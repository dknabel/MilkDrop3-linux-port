# MilkDrop3 Visualization Core Architecture Analysis

## Overview

MilkDrop3 is a portable music visualization application based on Ryan Geiss's original MilkDrop 2. The codebase is organized into:
- **Main visualization engine**: `/code/vis_milk2/` (~25,600 LOC)
- **Expression evaluator (EEL2)**: `/code/ns-eel2/` (Dynamic expression evaluation language)
- **Audio capture**: `/code/audio/` (Windows-specific audio APIs)
- **Plugin shell**: `/code/vis_milk2/pluginshell.cpp/h` (Winamp plugin interface)

---

## Directory Structure

```
MilkDrop3/
├── code/
│   ├── vis_milk2/              # Core visualization (vis_milk2.dll)
│   │   ├── plugin.cpp/h        # Main plugin class, frame rendering
│   │   ├── state.cpp/h         # Preset state container (parameters, expressions)
│   │   ├── pluginshell.cpp/h   # Winamp plugin interface (WINDOWS-SPECIFIC)
│   │   ├── dxcontext.cpp/h     # DirectX9 context management (WINDOWS-SPECIFIC)
│   │   ├── support.cpp/h       # D3D matrix/math utilities (WINDOWS-SPECIFIC)
│   │   ├── texmgr.cpp/h        # Texture management
│   │   ├── textmgr.cpp/h       # Text rendering (GDI-based, WINDOWS-SPECIFIC)
│   │   ├── menu.cpp/h          # UI menu system (WINDOWS-SPECIFIC)
│   │   ├── utility.cpp/h       # Helper functions
│   │   ├── state.cpp           # Preset I/O, blending, EEL compilation
│   │   ├── fft.cpp/h           # FFT audio analysis
│   │   ├── Milkdrop2PcmVisualizer.cpp  # PCM audio data handling
│   │   ├── milkdropfs.cpp      # Preset/file system operations (183 KB!)
│   │   ├── md_defines.h        # Constants & enums
│   │   ├── wasabi.h            # Winamp Wasabi UI wrapper (WINDOWS-SPECIFIC)
│   │   └── AutoWide.h, AutoCharFn.h  # Windows string conversion utilities
│   │
│   ├── ns-eel2/                # Nullsoft Expression Evaluator Library v2
│   │   ├── ns-eel.h            # Public API
│   │   ├── ns-eel-int.h        # Internal VM structures
│   │   ├── ns-eel-addfuncs.h   # Built-in functions
│   │   ├── nseel-compiler.c    # Expression compiler
│   │   ├── nseel-eval.c        # VM execution engine
│   │   ├── nseel-yylex.c       # Lexer
│   │   ├── nseel-caltab.c      # Call tables
│   │   ├── nseel-cfunc.c       # C function binding
│   │   ├── nseel-ram.c         # Memory management
│   │   ├── asm-nseel-x86-msvc.c    # x86 JIT compiler (WINDOWS)
│   │   ├── asm-nseel-x86-gcc.c     # x86 JIT compiler (UNIX)
│   │   └── asm-nseel-ppc-gcc.c     # PowerPC JIT compiler
│   │
│   └── audio/                  # Windows-specific audio capture
│       ├── loopback-capture.cpp/h  # WASAPI loopback audio
│       ├── audiobuf.cpp/h
│       └── prefs.cpp/h
│
└── linux/                      # Linux Wine compatibility guide
    └── MilkDrop 3 linux.exe    # Recompiled binary for Wine
```

---

## Core Visualization Pipeline

### 1. Main Plugin Class: `CPlugin` (`plugin.h/cpp`)
**File**: `/code/vis_milk2/plugin.cpp` (356 KB - the largest file)

**Key Responsibilities**:
- Frame rendering pipeline (`RenderFrame()`)
- Preset loading/blending (`LoadPreset()`, `LoadPresetTick()`)
- DirectX device management
- Shader compilation and management
- Input handling (keyboard/mouse)
- Audio analysis integration
- Waveform/shape rendering
- Texture management

**Key Methods**:
- `RenderFrame(int bRedraw)` - Main per-frame rendering
- `LoadPreset(const wchar_t *szPresetFilename, float fBlendTime)` - Load .milk file
- `LoadPresetTick()` - Progressive preset loading
- `RenderWaveform()` - Draw audio waveform
- `RenderShapes()` - Render custom shapes
- `RenderComposite()` - Final composite pass with pixel shaders

**DirectX Objects**:
- Vertex declarations (`m_pMyVertDecl`, `m_pWfVertDecl`, `m_pSpriteVertDecl`)
- Shader objects (warp & composite pixel shaders)
- Texture surfaces (render targets for blur chain)
- Fallback shaders for compatibility

---

### 2. Preset State: `CState` (`state.h/cpp`)
**File**: `/code/vis_milk2/state.cpp` (81 KB)

**Core Data Structure**:
Encapsulates all preset configuration. Contains:

```cpp
class CState {
    // Post-processing
    float m_fGammaAdj;
    float m_fVideoEchoZoom, m_fVideoEchoAlpha;
    
    // Wave parameters
    int m_nWaveMode;
    float m_fWaveAlpha, m_fWaveScale;
    
    // Map transformation (zoom, rotation, warp)
    float m_fZoom, m_fRot, m_fWarpAmount, m_fStretchX, m_fStretchY;
    
    // Colors and effects
    float m_fWaveR, m_fWaveG, m_fWaveB;
    float m_fOuterBorderSize, m_fOuterBorderR/G/B/A;
    
    // Custom shapes and waves
    CShape m_shape[MAX_CUSTOM_SHAPES];  // Up to 16 shapes (MD3 extended from 4)
    CWave  m_wave[MAX_CUSTOM_WAVES];    // Up to 4 waves
    
    // EEL2 compiled code
    NSEEL_CODEHANDLE m_pf_codehandle;   // Per-frame equations
    NSEEL_CODEHANDLE m_pp_codehandle;   // Per-pixel equations
    char m_szPerFrameInit[32KB];
    char m_szPerFrameExpr[32KB];
    char m_szPerPixelExpr[32KB];
    char m_szWarpShadersText[32KB];     // HLSL pixel shader
    char m_szCompShadersText[32KB];     // HLSL pixel shader
    
    // Blending support
    bool m_bBlending;
    float m_fBlendStartTime, m_fBlendDuration;
};
```

**Key Methods**:
- `Import(const wchar_t *szIniFile, ...)` - Load .milk preset file (INI format)
- `Export(const wchar_t *szIniFile)` - Save preset
- `Default(DWORD ApplyFlags)` - Initialize to defaults
- `Randomize(int nMode)` - Generate random parameters
- `RecompileExpressions(int flags)` - Compile EEL2 code
- `StartBlendFrom(CState *s_from, ...)` - Begin preset transition

**Blendable Types**: `CBlendableFloat` - Smoothly transitions parameter values over time

---

### 3. EEL2 Expression Evaluator (`/ns-eel2/`)
**API Header**: `/code/ns-eel2/ns-eel.h`

EEL2 is a lightweight expression language embedded in presets. It evaluates:
- **Per-frame code** (runs once per frame)
- **Per-pixel code** (runs per texel - very performance-sensitive)

**Key Components**:

| File | Purpose |
|------|---------|
| `nseel-compiler.c` (50 KB) | Parse expressions into VM bytecode |
| `nseel-eval.c` | Execute bytecode |
| `nseel-yylex.c` | Tokenizer/lexer |
| `nseel-caltab.c` | Call table generation |
| `asm-nseel-x86-msvc.c` | x86 JIT compiler (generates machine code) |
| `asm-nseel-x86-gcc.c` | x86 JIT for GCC |
| `nseel-ram.c` | Dynamic memory management for scripts |

**Public API**:
```cpp
NSEEL_init()                            // Initialize EEL2
NSEEL_VM_alloc()                        // Create a virtual machine
NSEEL_code_compile(ctx, code)           // Compile expression code
NSEEL_code_execute(handle)              // Run compiled code
NSEEL_VM_regvar(ctx, name)              // Register a variable
NSEEL_code_free(handle)                 // Free compiled code
NSEEL_VM_free(ctx)                      // Destroy VM
```

**EEL2 Syntax Example** (from presets):
```eel2
// Per-frame code:
q1 = bass * 0.5;
q2 = mid * 0.3;
cx = 0.5 + sin(time*0.5)*0.2;  // Center X oscillates

// Per-pixel code:
uv = (uv - 0.5) * (1.0 + zoom) + 0.5;
uv += sin(uv.x * 10 + time) * 0.1;
```

**Built-in Variables** (from CState registration):
- Audio: `bass`, `mid`, `treb` (0-1, normalized)
- Time: `time`, `fps`, `frame`
- Audio buckets: `q1`-`q64` (user-controlled)
- Temporary: `t1`-`t8`
- Shape/wave params: `sides`, `additive`, `x`, `y`, `rad`, `ang`, etc.

---

### 4. Preset File Format (.milk)
**Location**: `/code/vis_milk2/state.cpp::CState::Import()`
**Format**: INI-like text file with embedded code sections

**Structure**:
```
[PRESET]
MILKDROP_PRESET_VERSION=202
PSVERSION_WARP=2
PSVERSION_COMP=2

// Parameters (blendable values)
fDecay=0.95
fGammaAdj=1.0
fVideoEchoZoom=0.9
nWaveMode=3
bAdditiveWaves=1
...

// Custom shapes (up to 16 in MD3)
[shape_0_init]
sides=4;tex_zoom=1;additive=0;

[shape_0_per_frame]
sides = 4 + bass*8;
x = 0.5; y = 0.5;
...

// Per-frame expressions (affects global state)
[per_frame_init_1]
q1 = 0; q2 = 0; ...

[per_frame_eqs_0]
cx = 0.5;
cy = 0.5;
zoom = 1.0 + bass*0.3;
...

// Per-pixel expressions (warp texture coordinates)
[per_pixel_eqs_0]
uv = (uv - 0.5) * (1.0 + zoom*0.2) + 0.5;
uv += (mid*0.2) * sin(time*2);

// Pixel shaders (HLSL)
[warp_shader_code]
float2 warp(float2 tc, float time, float4 rand) {
    return tc + sin(tc.y*10)*0.02;
}

[comp_shader_code]
float3 composite(float3 tex, float3 wave, float3 warp) {
    return tex + wave*0.5;
}
```

**Key Sections**:
- `[PRESET]` - Metadata
- `[per_frame_init_*]` - Initialize once per frame
- `[per_frame_eqs_*]` - Per-frame calculation
- `[per_pixel_eqs_*]` - Per-pixel warping
- `[shape_N_init]`, `[shape_N_per_frame]` - Shape definitions (N=0..15)
- `[wave_N_init]`, `[wave_N_per_frame]` - Waveform definitions (N=0..3)
- `[warp_shader_code]` - Vertex/pixel shader (HLSL)
- `[comp_shader_code]` - Composite shader

**Parsing**: `CState::Import()` reads line-by-line, extracts parameters via `GetFastFloat()`, `GetFastInt()`, builds code strings for later compilation.

---

### 5. DSL Features: Shapes and Waves
**Definitions**: `/code/vis_milk2/state.h` (CShape, CWave classes)

#### CShape
```cpp
class CShape {
    int enabled, sides, additive, textured, instances;
    float x, y, rad, ang;          // Position, radius, angle
    float r, g, b, a;              // Primary color RGBA
    float r2, g2, b2, a2;          // Secondary color
    float border_r/g/b/a;          // Border color
    float tex_ang, tex_zoom;       // Texture mapping
    
    char m_szInit[32KB];           // Init code (EEL2)
    char m_szPerFrame[32KB];       // Per-frame code (EEL2)
    NSEEL_CODEHANDLE m_pf_codehandle;
    NSEEL_VMCTX m_pf_eel;          // EEL2 VM context
    
    // Variables exposed to EEL2:
    double *var_pf_sides, *var_pf_textured, *var_pf_additive;
    double *var_pf_x, *var_pf_y, *var_pf_rad, *var_pf_ang;
    double *var_pf_r, *var_pf_g, *var_pf_b, *var_pf_a;
};
```

**Rendering**: Rendered as polygon with 3..500 sides (MD3 extends from 100).

#### CWave
```cpp
class CWave {
    int enabled, mode, useDots, thick, additive, brighten;
    float r, g, b, a;              // Color RGBA
    
    char m_szInit[32KB];
    char m_szPerPoint[32KB];       // Per-point code (EEL2)
    NSEEL_CODEHANDLE m_pf_codehandle;
    // ... similar to CShape
};
```

**Rendering**: Polyline drawn over audio waveform.

---

## Windows-Specific Code (To Skip)

### DirectX 9 Rendering
**Files**: 
- `/code/vis_milk2/dxcontext.cpp/h` (6 KB)
- `/code/vis_milk2/support.cpp/h` (14 KB)
- Parts of `plugin.cpp` (shader compilation, texture management, device creation)

**Classes**:
- `DXContext` - DirectX device wrapper
- `PShaderSet`, `VShaderSet` - Shader container structs
- `TexInfo` - Texture metadata

**Concepts**:
- D3DXMATRIX, D3DXVECTOR3/4 - Math vectors
- IDirect3DDevice9 - GPU device
- D3D_VERTEX_DECLARATION - Vertex format
- HLSL shader compilation via D3DX9

---

### Winamp Plugin Shell
**Files**:
- `/code/vis_milk2/pluginshell.cpp/h` (89 KB)
- `/code/vis_milk2/plugin.h` (plugin registration)

**Concept**: CPlugin extends CPluginShell, which handles Winamp IPC messages, audio delivery, window creation. Inherits from `IPC_PLUGIN_API` (Winamp plugin interface).

---

### Win32/GDI UI
**Files**:
- `/code/vis_milk2/menu.cpp/h` (23 KB)
- `/code/vis_milk2/textmgr.cpp/h` (30 KB)
- `/code/vis_milk2/wasabi.h` - Winamp Wasabi dialog API

**Concept**: HWND-based menus, GDI font rendering, MessageBox dialogs.

---

### Audio Capture
**Files**: `/code/audio/` (Windows WASAPI loopback)
- `loopback-capture.cpp/h` - WASAPI audio endpoint enumeration
- `audiobuf.cpp` - Audio ring buffer
- `prefs.cpp` - Audio preferences

**Alternative**: MilkDrop3 also reads from Winamp playback or external audio sources via Win32 APIs.

---

## Platform-Agnostic Core Files

### Tier 1: Absolutely Essential
These files contain zero Windows dependencies and are the true core:

| File | Size | Purpose |
|------|------|---------|
| `state.cpp/h` | 81 KB | Preset parsing, state management |
| `Milkdrop2PcmVisualizer.cpp` | 25 KB | Audio data parsing |
| `fft.cpp/h` | 11 KB | FFT audio analysis |
| `utility.cpp/h` | 28 KB | String, math, file utilities |
| `ns-eel2/*.c` | 200+ KB | Expression evaluator |

### Tier 2: Rendering Abstraction
These would need replacement with graphics backend of choice:

- `texmgr.cpp/h` - Texture management (currently D3D)
- `support.cpp/h` - Matrix/transform math (D3DX9)
- Parts of `plugin.cpp` - Core rendering pipeline

---

## Key Data Flows

### Preset Loading Flow
```
1. User presses 'L' or calls LoadPreset(filename, blendTime)
2. CState::Import(filename) reads .milk file
   - Parses INI sections for parameters
   - Accumulates EEL2 code strings
3. CState::RecompileExpressions()
   - Calls NSEEL_code_compile() for per-frame, per-pixel code
   - Calls NSEEL_code_compile() for shape/wave init+per-frame code
4. If blendTime > 0:
   - StartBlendFrom() begins smooth transition
   - Each frame, CBlendableFloat::eval(time) interpolates values
5. RenderFrame() runs each loop:
   - NSEEL_code_execute(per_frame_code)  // Update q1..q64, time, etc.
   - Update shapes and waves
   - Render waveform, shapes
   - Apply warp shader (per-pixel)
   - Apply composite shader
   - Video echo, decay
```

### Per-Frame Expression Execution
```
CPlugin::RenderFrame()
  ├─ Update audio analysis (bass, mid, treb via FFT)
  ├─ NSEEL_code_execute(m_pState->m_pf_codehandle)
  │   └─ Reads: bass, mid, treb, time, fps, frame, q[1..64], t[1..8]
  │   └─ Writes: zoom, rot, cx, cy, warp, decay, etc.
  ├─ For each shape:
  │   └─ NSEEL_code_execute(shape->m_pf_codehandle)
  │       └─ Updates: x, y, rad, ang, sides, r, g, b, a
  ├─ For each wave:
  │   └─ NSEEL_code_execute(wave->m_pf_codehandle)
  │       └─ Updates: r, g, b, a
  └─ Render pipeline:
      ├─ Render waveform (from audio PCM data)
      ├─ Render shapes
      ├─ Set warp shader constants (zoom, rot, cx, cy, warp_amount, etc.)
      ├─ Execute per-pixel shader (warp texture coordinates)
      ├─ Render quad with warped texture
      └─ Composite with previous frame (video echo, decay)
```

---

## Code Size Breakdown

| Component | Size | Notes |
|-----------|------|-------|
| `plugin.cpp` | 356 KB | Monolithic; contains rendering, shader mgmt, I/O |
| `milkdropfs.cpp` | 183 KB | File system, preset directory scanning |
| `state.cpp` | 81 KB | Preset I/O, EEL compilation, parameter blending |
| `pluginshell.cpp` | 89 KB | Winamp plugin shell (Windows) |
| `ns-eel2/*.c` | 200+ KB | Expression evaluator (C, mostly portable) |
| `textmgr.cpp` | 30 KB | Text rendering (GDI, Windows) |
| `utility.cpp` | 28 KB | Helpers (some platform-specific string funcs) |
| `menu.cpp` | 23 KB | Menu UI (Windows) |
| **Total** | ~970 KB | (Excluding headers, audio, DX9 layer) |

---

## Extraction Strategy for `molkdroop`

### Recommended Modular Breakdown

#### Core Module: `src/core/expression/`
- Copy `/code/ns-eel2/` almost verbatim
- Update #includes and linkage
- Keep x86 assembly, but make it optional (no JIT = slower but portable)
- Test: `NSEEL_code_compile()` and `NSEEL_code_execute()` on simple expressions

#### Core Module: `src/core/preset/`
- Rewrite `state.cpp::CState::Import()` to use portable file I/O
- Implement INI parser from scratch (simple state machine)
- Extract preset parameter parsing logic
- Keep `state.h` struct definitions as-is
- Remove DirectX types, replace with graphics-agnostic equivalents

#### Core Module: `src/core/audio/`
- Extract `fft.cpp` (no Win32 deps)
- Extract PCM visualization logic from `Milkdrop2PcmVisualizer.cpp`
- Define abstract audio input interface (samples, sample rate)
- Stub out WASAPI/loopback code

#### Core Module: `src/core/rendering/`
- **NEW**: Abstract graphics backend interface
  - RenderTarget, Shader, Texture, Sampler, VertexBuffer traits
  - Waveform rendering (mesh generation)
  - Shape rendering (polygon generation)
  - Per-pixel coordinate warping logic
- Extract `plugin.cpp` render pipeline, removing D3D calls
- Use abstraction for texture upload, shader compilation

#### Separate (Don't extract yet):
- `/code/vis_milk2/dxcontext.cpp/h` → Implement for D3D backend only
- `/code/vis_milk2/menu.cpp/h` → Implement per-UI framework
- `/code/vis_milk2/textmgr.cpp/h` → Implement per-graphics backend

---

## DSL (Domain-Specific Language) Summary

### EEL2 Preset DSL

**Scope**: Mathematical expressions evaluated in C-like syntax.

**Grammar** (simplified):
```
expression := assignment | ternary | call | binary_op | unary_op | literal | variable
assignment := variable = expression
ternary := condition ? true_expr : false_expr
call := function_name(arg1, arg2, ...)
binary_op := expression +|-|*|/|%|< |>|==|!=|&&|||...  expression
unary_op := -|!  expression
literal := number | "string"
variable := identifier | identifier[index]
```

**Built-in Functions** (from `ns-eel-addfuncs.h`):
- Math: `sin()`, `cos()`, `tan()`, `asin()`, `acos()`, `atan()`, `atan2()`, `abs()`, `sqrt()`, `pow()`, `exp()`, `log()`, `ceil()`, `floor()`, `min()`, `max()`
- Array: `getpixel()`, `setpixel()`
- Control: `if()...else`, `while()`, `loop()`

**Pre-Defined Variables** (per context):
- **Audio**: `bass`, `mid`, `treb`, `bass_att`, `mid_att`, `treb_att`
- **Timing**: `time`, `fps`, `frame`
- **User**: `q1`..`q64`, `t1`..`t8`
- **Mesh**: `meshx`, `meshy`
- **Shape params**: `sides`, `x`, `y`, `rad`, `ang`, `r`, `g`, `b`, `a`, `additive`, `textured`, etc.
- **Global state**: `zoom`, `rot`, `cx`, `cy`, `warp`, `decay`, `brighten`, etc.

**Execution Contexts**:
1. **Per-Frame Init** (`[per_frame_init_*]`) - Runs once, initializes q variables
2. **Per-Frame** (`[per_frame_eqs_*]`) - Runs every frame, reads audio, updates state
3. **Per-Pixel** (`[per_pixel_eqs_*]`) - Runs per texel, applies warping/distortion
4. **Shape/Wave Init** (`[shape_N_init]`, `[wave_N_init]`) - Initialize custom geometry
5. **Shape/Wave Per-Frame** (`[shape_N_per_frame]`, `[wave_N_per_frame]`) - Update geometry each frame

**Performance Note**: Per-pixel code is JIT-compiled to native x86 for speed. Per-frame code is usually interpreted bytecode.

---

## Example Preset Structure

```milk
; Simple zoom-on-beat preset
[PRESET]
MILKDROP_PRESET_VERSION=202
PSVERSION_WARP=2
PSVERSION_COMP=2
fDecay=0.95
fGammaAdj=1.0
nWaveMode=1
bAdditiveWaves=1
bBrighten=0

[per_frame_init_1]
q1 = 0; q2 = 0; beat_time = 0;

[per_frame_eqs_0]
; Detect beat
q1 = bass - q1 * 0.7;
beat = q1 > 0.3 ? 1 : 0;
beat_time = beat ? time : beat_time;

; Zoom out on beat, then slowly zoom back in
beat_zoom = beat ? 1.5 : 0.0;
zoom = 1.0 + beat_zoom * exp(-(time - beat_time) * 2.0);

; Rotate slowly
rot = time * 0.3;

[per_pixel_eqs_0]
; Swirl effect
uv = (uv - 0.5) * (1.0 - zoom*0.1) + 0.5;
angle = atan2(uv.y - 0.5, uv.x - 0.5) + rot * 0.1;
dist = length(uv - 0.5);
uv = 0.5 + vec2(cos(angle), sin(angle)) * dist;

[shape_0_init]
sides = 6; tex_zoom = 1; additive = 1;

[shape_0_per_frame]
x = 0.5 + sin(time)*0.2;
y = 0.5 + cos(time*0.7)*0.2;
rad = 0.3 + mid*0.2;
sides = 6 + floor(treb*10);
r = sin(time)*0.5 + 0.5;
g = cos(time*0.8)*0.5 + 0.5;
b = sin(time*1.2)*0.5 + 0.5;
a = bass;

[warp_shader_code]
float2 warp_main(float2 uv) {
    return uv;  // Identity (warping done in per-pixel)
}

[comp_shader_code]
float3 comp_main(float3 tex, float3 wave) {
    return tex + wave * 0.5;
}
```

---

## Summary: Files to Extract

### Definitely Extract (Core)
```
src/core/expression/
  ├─ ns-eel.h
  ├─ ns-eel-int.h
  ├─ ns-eel-addfuncs.h
  ├─ nseel-compiler.c
  ├─ nseel-eval.c
  ├─ nseel-yylex.c
  ├─ nseel-caltab.c
  ├─ nseel-cfunc.c
  ├─ nseel-ram.c
  └─ asm-nseel-[arch].c  (all variants)

src/core/preset/
  ├─ state.h  (modified to remove D3D types)
  ├─ preset-parser.cpp  (extracted from state.cpp)
  └─ preset-types.h  (CState, CShape, CWave, etc.)

src/core/audio/
  ├─ fft.cpp/h
  ├─ pcm-analyzer.cpp/h  (extracted from Milkdrop2PcmVisualizer.cpp)
  └─ audio-interface.h  (abstract input)

src/core/rendering/
  ├─ waveform-generator.cpp/h
  ├─ shape-generator.cpp/h
  ├─ visualization-context.h  (abstract graphics)
  └─ render-pipeline.cpp/h  (extracted from plugin.cpp)
```

### Skip (Windows-Specific)
```
/code/vis_milk2/dxcontext.cpp/h
/code/vis_milk2/pluginshell.cpp/h
/code/vis_milk2/menu.cpp/h
/code/vis_milk2/textmgr.cpp/h
/code/vis_milk2/wasabi.h
/code/audio/*  (WASAPI, Win32)
```

### Adapt/Rewrite
```
plugin.cpp  → Refactor into modular pipeline
support.cpp → Extract portable math, rewrite D3D-specific code
utility.cpp → Keep, adapt Win32 string functions
```

---

## Next Steps

1. **Stage 1**: Set up build system, integrate EEL2 unchanged
2. **Stage 2**: Extract CState preset parser, write INI reader
3. **Stage 3**: Implement abstract graphics backend trait
4. **Stage 4**: Extract render pipeline, remove D3D calls
5. **Stage 5**: Implement WebGL/Metal/Vulkan backends

