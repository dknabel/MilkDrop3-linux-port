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
