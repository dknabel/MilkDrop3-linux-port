// src/audio/factory.cpp
#include "../platform/audio.h"

#ifdef HAVE_PIPEWIRE
#include "pipewire_input.h"
#endif

#ifdef HAVE_PULSEAUDIO
#include "pulseaudio_input.h"
#endif

#include <iostream>

AudioInput* createAudioInput() {
#ifdef HAVE_PIPEWIRE
  try {
    auto* input = new PipeWireInput();
    std::cout << "Created PipeWire audio input\n";
    return input;
  } catch (const std::exception& e) {
    std::cerr << "Failed to create PipeWire audio input: " << e.what() << "\n";
#ifdef HAVE_PULSEAUDIO
    std::cout << "Falling back to PulseAudio\n";
#else
    return nullptr;
#endif
  }
#endif

#ifdef HAVE_PULSEAUDIO
  try {
    auto* input = new PulseAudioInput();
    std::cout << "Created PulseAudio audio input\n";
    return input;
  } catch (const std::exception& e) {
    std::cerr << "Failed to create PulseAudio audio input: " << e.what() << "\n";
    return nullptr;
  }
#endif

  std::cerr << "No audio backend available (HAVE_PIPEWIRE or HAVE_PULSEAUDIO not defined)\n";
  return nullptr;
}
