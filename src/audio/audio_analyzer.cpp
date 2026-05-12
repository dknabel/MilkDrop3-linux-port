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

  // Initialize KissFFT
  fftConfig_ = kiss_fft_alloc(fftSize, 0, nullptr, nullptr);
  fftInput_.resize(fftSize);
  fftOutput_.resize(fftSize);
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
    // Pad with zeros if not enough samples
    std::copy(samples.begin(), samples.end(), buffer_.begin());
    std::fill(buffer_.begin() + samples.size(), buffer_.end(), 0.0f);
  } else {
    std::copy(samples.begin(), samples.begin() + fftSize_, buffer_.begin());
  }

  // Apply window
  applyWindow(buffer_);

  // Compute FFT using KissFFT
  computeFFT();

  // Extract magnitude spectrum
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

void AudioAnalyzer::computeFFT() {
  // Convert float samples to complex format
  for (int i = 0; i < fftSize_; ++i) {
    fftInput_[i].r = buffer_[i];
    fftInput_[i].i = 0.0f;
  }

  // Perform FFT
  kiss_fft(fftConfig_, fftInput_.data(), fftOutput_.data());
}
