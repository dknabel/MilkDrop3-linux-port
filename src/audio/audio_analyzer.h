// src/audio/audio_analyzer.h
#pragma once
#include <vector>
#include <complex>
#include <kiss_fft.h>

class AudioAnalyzer {
public:
  AudioAnalyzer(int fftSize = 512);
  ~AudioAnalyzer();

  // Input: raw audio samples
  // Output: frequency bins (magnitude spectrum)
  std::vector<float> analyze(const std::vector<float>& samples);

  int getFrequencyBinCount() const { return fftSize_ / 2; }

private:
  int fftSize_;
  std::vector<float> window_;
  std::vector<float> buffer_;

  // KissFFT members
  kiss_fft_cfg fftConfig_;
  std::vector<kiss_fft_cpx> fftInput_;
  std::vector<kiss_fft_cpx> fftOutput_;

  void applyWindow(std::vector<float>& samples);
  void computeFFT();
};
