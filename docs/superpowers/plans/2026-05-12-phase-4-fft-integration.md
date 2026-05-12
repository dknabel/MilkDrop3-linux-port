# Phase 4.1: FFT Integration Implementation Plan

> **For agentic workers:** Use superpowers:subagent-driven-development to execute this plan task-by-task.

**Goal:** Replace placeholder FFT with real frequency analysis library (KissFFT). Enable audio → frequency bins pipeline.

**Architecture:** Use KissFFT (lightweight, C-based, perfect for audio visualization). Integrate into existing AudioAnalyzer class.

**Tech Stack:** KissFFT (3.0+), C++17, CMake

---

## File Structure

**To be added/modified:**
```
include/
  └── kiss_fft/               (KissFFT headers - will be downloaded)
src/audio/
  └── audio_analyzer.cpp      (integrate KissFFT)
CMakeLists.txt                (add KissFFT fetch/build)
src/CMakeLists.txt            (link KissFFT)
```

---

## Phase 4.1 Tasks

### Task 1: Add KissFFT via CMake (FetchContent)

**Files:**
- Modify: `CMakeLists.txt`

**Specification:**

Add to root CMakeLists.txt after existing dependencies:

```cmake
# KissFFT for audio analysis
include(FetchContent)
FetchContent_Declare(
  kissfft
  URL https://github.com/mborgerding/kissfft/archive/refs/tags/v131.tar.gz
  URL_HASH SHA256=da0ce21161e4df1677da79654e5599abe5ceee1f373c3b434c22e79ad0a87e5a
)
FetchContent_MakeAvailable(kissfft)
```

After `add_subdirectory(src)`, add:

```cmake
# Link KissFFT to milkdrop3 target
target_link_libraries(milkdrop3 kissfft::kissfft)
target_include_directories(milkdrop3 PRIVATE ${kissfft_SOURCE_DIR})
```

Test CMake configuration runs without errors.

---

### Task 2: Implement KissFFT Integration in AudioAnalyzer

**Files:**
- Modify: `src/audio/audio_analyzer.h`
- Modify: `src/audio/audio_analyzer.cpp`

**Specification:**

Update `audio_analyzer.h` to include KissFFT headers and store kiss_fft_state:

```cpp
// src/audio/audio_analyzer.h
#pragma once
#include <vector>
#include <complex>
#include <kiss_fft.h>

class AudioAnalyzer {
public:
  AudioAnalyzer(int fftSize = 512);
  ~AudioAnalyzer();
  
  std::vector<float> analyze(const std::vector<float>& samples);
  
  int getFrequencyBinCount() const { return fftSize_ / 2; }
  
private:
  int fftSize_;
  std::vector<float> window_;
  std::vector<float> buffer_;
  kiss_fft_cfg fftConfig_;
  std::vector<kiss_fft_cpx> fftInput_;
  std::vector<kiss_fft_cpx> fftOutput_;
  
  void applyWindow(std::vector<float>& samples);
  void computeFFT(std::vector<float>& samples);
};
```

Update `audio_analyzer.cpp` to use KissFFT:

```cpp
// src/audio/audio_analyzer.cpp
#include "audio_analyzer.h"
#include <cmath>
#include <algorithm>

AudioAnalyzer::AudioAnalyzer(int fftSize)
  : fftSize_(fftSize) {
  
  // Hann window
  window_.resize(fftSize);
  for (int i = 0; i < fftSize; ++i) {
    window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fftSize - 1)));
  }
  
  // KissFFT setup
  fftConfig_ = kiss_fft_alloc(fftSize, 0, nullptr, nullptr);
  fftInput_.resize(fftSize);
  fftOutput_.resize(fftSize);
  
  buffer_.resize(fftSize, 0.0f);
}

AudioAnalyzer::~AudioAnalyzer() {
  if (fftConfig_) {
    kiss_fft_free(fftConfig_);
  }
}

std::vector<float> AudioAnalyzer::analyze(const std::vector<float>& samples) {
  // Shift buffer and add new samples
  int samples_to_add = std::min((int)samples.size(), fftSize_);
  
  if (samples.size() < fftSize_) {
    std::copy(samples.begin(), samples.end(), buffer_.begin());
    std::fill(buffer_.begin() + samples.size(), buffer_.end(), 0.0f);
  } else {
    std::copy(samples.begin(), samples.begin() + fftSize_, buffer_.begin());
  }
  
  // Apply window
  applyWindow(buffer_);
  
  // Compute FFT
  computeFFT(buffer_);
  
  // Extract magnitude spectrum (frequency bins)
  std::vector<float> freqBins(fftSize_ / 2);
  for (int i = 0; i < fftSize_ / 2; ++i) {
    float real = fftOutput_[i].r;
    float imag = fftOutput_[i].i;
    freqBins[i] = std::sqrt(real * real + imag * imag) / fftSize_;
  }
  
  return freqBins;
}

void AudioAnalyzer::applyWindow(std::vector<float>& samples) {
  for (int i = 0; i < (int)samples.size(); ++i) {
    samples[i] *= window_[i];
  }
}

void AudioAnalyzer::computeFFT(std::vector<float>& samples) {
  // Convert float samples to complex input for KissFFT
  for (int i = 0; i < fftSize_; ++i) {
    fftInput_[i].r = samples[i];
    fftInput_[i].i = 0.0f;
  }
  
  // Run FFT
  kiss_fft(fftConfig_, fftInput_.data(), fftOutput_.data());
}
```

---

### Task 3: Update CMakeLists.txt Target Linking

**Files:**
- Modify: `src/CMakeLists.txt`

**Specification:**

After `add_executable(milkdrop3 ...)`, add:

```cmake
# Link KissFFT
target_link_libraries(milkdrop3 kissfft::kissfft)
target_include_directories(milkdrop3 PRIVATE ${kissfft_SOURCE_DIR})
```

Ensure audio_analyzer.cpp is in MILKDROP_SOURCES (already is).

---

### Task 4: Test Compilation and Frequency Analysis

**Files:**
- Test: Build and verify FFT integration

**Specification:**

```bash
cd /home/drew/Documents/molkdroop/build
cmake ..
make
```

Expected: Successful compilation with KissFFT linked.

**Verification:**
- No undefined reference errors to kiss_fft symbols
- audio_analyzer.h includes kiss_fft.h without errors
- Executable links successfully

---

## Success Criteria

- [ ] KissFFT fetched and available in build
- [ ] AudioAnalyzer uses real FFT via KissFFT
- [ ] analyze() returns proper magnitude spectrum (not just windowed samples)
- [ ] Code compiles without errors
- [ ] Frequency bin count correct: fftSize / 2 bins
- [ ] Memory properly managed (kiss_fft_free called)

---

## Notes

- KissFFT is C-based, C++-compatible
- kiss_fft_alloc/free handle memory for FFT state
- Output magnitude computed as sqrt(real^2 + imag^2) / fftSize for normalization
- Window still applied before FFT for spectral leakage reduction

