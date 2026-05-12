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
  AudioInput* pwInput = new PipeWireInput();
  if (pwInput->initialize("default")) {
    std::cout << "PipeWire initialized successfully\n";
    return pwInput;
  } else {
    std::cout << "PipeWire initialization failed, trying PulseAudio...\n";
    delete pwInput;
  }
#endif

#ifdef HAVE_PULSEAUDIO
  std::cout << "Creating PulseAudio audio input...\n";
  AudioInput* paInput = new PulseAudioInput();
  if (paInput->initialize("default")) {
    std::cout << "PulseAudio initialized successfully\n";
    return paInput;
  } else {
    std::cerr << "PulseAudio initialization failed\n";
    delete paInput;
  }
#endif

  std::cout << "No audio backend available\n";
  return nullptr;
}
