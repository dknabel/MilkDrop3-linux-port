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
