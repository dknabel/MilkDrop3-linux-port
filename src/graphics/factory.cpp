// src/graphics/factory.cpp
#include "../platform/graphics.h"
#include "opengl_device.h"

GraphicsDevice* createGraphicsDevice() {
  return new OpenGLDevice();
}

void deleteGraphicsDevice(GraphicsDevice* device) {
  delete device;
}
