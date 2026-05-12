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
  std::cout << "Creating PipeWire audio input...\n";
  auto* input = new PipeWireInput();
  if (input->initialize("default")) {
    std::cout << "PipeWire initialized successfully\n";
    return input;
  } else {
    std::cout << "PipeWire initialization failed, trying PulseAudio...\n";
    delete input;
  }
#endif

#ifdef HAVE_PULSEAUDIO
  std::cout << "Creating PulseAudio audio input...\n";
  auto* input = new PulseAudioInput();
  if (input->initialize("default")) {
    std::cout << "PulseAudio initialized successfully\n";
    return input;
  } else {
    std::cerr << "PulseAudio initialization failed\n";
    delete input;
  }
#endif

  std::cout << "No audio backend available\n";
  return nullptr;
}
