// src/graphics/opengl_device.h
#pragma once
#include "../platform/graphics.h"
#include "../platform/window.h"
#include "shader.h"
#include <glad/glad.h>
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

  // Helper methods for command execution
  void executeClearCommand(const RenderCommand& cmd);
  void executeDrawWaveformCommand(const RenderCommand& cmd);
  void executeDrawShapeCommand(const RenderCommand& cmd);

  // Waveform VAO/VBO for efficient rendering
  GLuint waveformVAO_ = 0;
  GLuint waveformVBO_ = 0;
  int waveformVertexCount_ = 0;

  // Shape VAO/VBO for efficient rendering
  GLuint shapeVAO_ = 0;
  GLuint shapeVBO_ = 0;

  // Default shader for simple rendering
  Shader* defaultShader_ = nullptr;

  // Default shader sources
  struct DefaultShaderSource {
    static constexpr std::string_view vertexShader = R"(
#version 330 core
layout(location = 0) in vec2 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
  gl_Position = projection * view * model * vec4(position, 0.0, 1.0);
}
)";

    static constexpr std::string_view fragmentShader = R"(
#version 330 core
out vec4 FragColor;

uniform vec4 color;

void main() {
  FragColor = color;
}
)";
  };
};
