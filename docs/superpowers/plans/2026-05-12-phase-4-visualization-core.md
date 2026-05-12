# Phase 4.3: Milkdrop3 Visualization Core Integration

> **For agentic workers:** Use superpowers:subagent-driven-development to execute this plan task-by-task.

**Goal:** Integrate Milkdrop3 visualization engine. Enable preset loading, DSL interpretation, and render command generation.

**Architecture:** Extract core visualization from Milkdrop3 repo, adapt to Linux port architecture, wire frequency data through DSL interpreter to render output.

**Tech Stack:** Milkdrop3 core (C++), existing platform abstractions

---

## Context

Milkdrop3 source: https://github.com/milkdrop2077/MilkDrop3

The visualization core includes:
- Preset DSL parser & interpreter
- Animation equation solver
- Shape/waveform morphing
- Blending mode handling
- Render command generation

---

## Phase 4.3 Tasks

### Task 1: Analyze Milkdrop3 Visualization Core Structure

**Objective:** Understand Milkdrop3 source layout, identify core visualization components.

**Action:**
1. Clone/download Milkdrop3 repo (if not already in environment)
2. Explore directory structure, identify:
   - Preset parser code (*.cpp/*.h)
   - DSL interpreter (animation equations)
   - Render command generation
   - Data structures for visualizations
3. Document findings: Which files are core visualization vs. platform-specific (Windows/Direct3D)?

**Expected output:**
- List of core visualization files to integrate
- Identify files that are Windows-specific (skip these)
- Map preset DSL features to rendering pipeline

**Scope:** This is analysis/research. Goal is understanding, not implementation yet.

---

### Task 2: Create Visualization Engine Wrapper

**Files:**
- Create: `src/core/visualization.h` - Abstract visualization interface
- Modify: `src/core/visualization.cpp` - Adapter/wrapper code

**Specification:**

```cpp
// src/core/visualization.h
#pragma once
#include "../platform/types.h"
#include <string>
#include <vector>

class VisualizationEngine {
public:
  VisualizationEngine();
  ~VisualizationEngine();
  
  // Load and execute a preset
  bool loadPreset(const std::string& presetContent);
  
  // Update with audio data and elapsed time
  void update(const std::vector<float>& frequencyBins, float deltaTime);
  
  // Get pending render commands
  std::vector<RenderCommand> getRenderCommands();
  
  // Reset animation state
  void reset();
  
private:
  // Milkdrop3 core components will go here
  // (populated in Task 3)
};
```

Implementation:
- Wrapper that instantiates Milkdrop3 core components
- Provides interface between audio pipeline and rendering
- Manages lifetime of internal DSL interpreter/renderer

---

### Task 3: Integrate Milkdrop3 Preset Parser

**Files:**
- Modify: `src/core/visualization.cpp` - Integrate preset parsing

**Specification:**

This task integrates the Milkdrop3 preset DSL parser:
1. Include necessary Milkdrop3 headers for preset parsing
2. In `loadPreset()`:
   - Parse preset file content (DSL format)
   - Initialize animation state
   - Extract initial parameters
3. Handle parsing errors gracefully
4. Store parsed preset state for update loop

Implementation approach:
- Use Milkdrop3's built-in preset parser (from integrated source)
- Map preset parameters to internal data structures
- Prepare for DSL equation evaluation in update()

---

### Task 4: Implement Update & Render Command Generation

**Files:**
- Modify: `src/core/visualization.cpp` - Implement update() and command generation

**Specification:**

In `VisualizationEngine::update()`:
1. Update timing and beat detection (if available)
2. Evaluate all DSL equations:
   - Animation equations (motion, rotation, scaling)
   - Waveform morphing (based on frequency bins)
   - Parameter interpolation
3. Generate render commands:
   - Clear screen (if needed)
   - Draw waveforms
   - Apply blending modes
   - Render shapes/primitives
4. Store commands in vector for retrieval

In `getRenderCommands()`:
- Return queued commands
- Clear queue after retrieval (prevents double-rendering)

In `reset()`:
- Clear animation state
- Reset all equations to defaults
- Prepare for new preset

---

### Task 5: Wire Visualization into Main Event Loop

**Files:**
- Modify: `src/main.cpp` - Integrate VisualizationEngine

**Specification:**

In `Milkdrop3Application` class:
1. Add member: `std::unique_ptr<VisualizationEngine> visualizer_;`
2. In `initialize()`:
   - Create visualizer_
   - Load first preset from preset manager
3. In `update()`:
   - After frequency analysis, call: `visualizer_->update(freqBins, deltaTime);`
4. In `render()`:
   - Get render commands: `auto cmds = visualizer_->getRenderCommands();`
   - Execute each command via graphics device
   - Example: `for (const auto& cmd : cmds) { graphics_->executeRenderCommand(cmd); }`

---

### Task 6: Implement OpenGL Render Command Execution

**Files:**
- Modify: `src/graphics/opengl_device.cpp` - Fill in executeRenderCommand()

**Specification:**

In `OpenGLDevice::executeRenderCommand()`:
1. Switch on command type (shader vs. geometry rendering)
2. For shader-based commands:
   - Get shader handle from command
   - Bind shader: `shader->use()`
   - Set uniforms (frequency data, time, parameters)
   - Bind geometry (VAO/VBO)
   - Draw with glDrawArrays/glDrawElements
3. For geometry commands:
   - Create temporary VAO/VBO if needed
   - Draw primitive (triangle, quad, line)
4. Apply blending mode from command

Note: This requires some geometry/VAO setup. Can start simple with full-screen quads.

---

## Success Criteria

- [ ] Milkdrop3 core identified and documented
- [ ] VisualizationEngine wrapper created
- [ ] Preset DSL parsing integrated
- [ ] Update/render command pipeline working
- [ ] Main loop wired to visualization engine
- [ ] Render commands executed (at least simple ones)
- [ ] Can load and display a preset (even static)
- [ ] No crashes on audio input

---

## Challenges & Mitigations

| Challenge | Mitigation |
|-----------|-----------|
| Milkdrop3 source is Windows-centric | Extract platform-agnostic core, skip Direct3D/Windows code |
| DSL parser is complex | Use existing parser from source, don't rewrite |
| Render commands need careful mapping | Define RenderCommand struct to be general enough |
| Performance of DSL interpreter | Profile early, optimize hot paths |
| Preset format variations | Handle parsing errors gracefully, provide fallback |

---

## Notes

- Phase 4.3 is larger than 4.1/4.2; expect 1-2 weeks of work
- Milkdrop3 source is modular enough that core visualization is separable from Windows platform code
- Start with simple presets (no complex equations) to verify pipeline
- Can optimize DSL interpreter later; correctness first
- RenderCommand struct may need expansion for all preset features

