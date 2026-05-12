// src/platform/graphics.h
#pragma once
#include <string>
#include <cstdint>
#include "types.h"

// Forward declaration
class Window;

class Shader {
public:
  virtual ~Shader() = default;

  virtual uint32_t getHandle() const = 0;
  virtual bool isValid() const = 0;
  virtual std::string getErrorLog() const = 0;
};

class Texture {
public:
  virtual ~Texture() = default;

  virtual uint32_t getHandle() const = 0;
  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
};

class GraphicsDevice {
public:
  virtual ~GraphicsDevice() = default;

  virtual bool initialize(Window* window) = 0;
  virtual void shutdown() = 0;

  virtual Shader* createShader(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
  virtual void deleteShader(Shader* shader) = 0;

  virtual Texture* createTexture(int width, int height, const void* data) = 0;
  virtual void deleteTexture(Texture* texture) = 0;

  virtual void clear(float r, float g, float b, float a) = 0;
  virtual void executeRenderCommand(const RenderCommand& cmd) = 0;
  virtual void present() = 0;
};

// Factory functions
GraphicsDevice* createGraphicsDevice();
void deleteGraphicsDevice(GraphicsDevice* device);
