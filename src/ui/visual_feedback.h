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
