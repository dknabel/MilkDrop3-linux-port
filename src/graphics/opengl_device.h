// src/graphics/opengl_device.h
#pragma once
#include "../platform/graphics.h"
#include "../platform/window.h"
#include "shader.h"
#include <unordered_map>
#include <memory>

class OpenGLTexture : public Texture {
public:
  OpenGLTexture(int width, int height, const void* data);
  ~OpenGLTexture();

  uint32_t getHandle() const override { return textureHandle_; }
  int getWidth() const override { return width_; }
  int getHeight() const override { return height_; }

private:
  uint32_t textureHandle_;
  int width_;
  int height_;
};

class OpenGLDevice : public GraphicsDevice {
public:
  OpenGLDevice();
  ~OpenGLDevice();

  bool initialize(Window* window) override;
  void shutdown() override;

  Shader* createShader(const std::string& vertexSrc, const std::string& fragmentSrc) override;
  void deleteShader(Shader* shader) override;

  Texture* createTexture(int width, int height, const void* data) override;
  void deleteTexture(Texture* texture) override;

  void clear(float r, float g, float b, float a) override;
  void executeRenderCommand(const RenderCommand& cmd) override;
  void present() override;

private:
  Window* window_;
  bool isInitialized_;

  bool initializeGLAD();
};
