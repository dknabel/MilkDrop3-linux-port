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
