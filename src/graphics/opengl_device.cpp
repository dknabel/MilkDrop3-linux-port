// src/graphics/opengl_device.cpp
#include "opengl_device.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// OpenGLTexture implementation
OpenGLTexture::OpenGLTexture(int width, int height, const void* data)
  : textureHandle_(0), width_(width), height_(height) {

  glGenTextures(1, &textureHandle_);
  glBindTexture(GL_TEXTURE_2D, textureHandle_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glBindTexture(GL_TEXTURE_2D, 0);
}

OpenGLTexture::~OpenGLTexture() {
  if (textureHandle_) {
    glDeleteTextures(1, &textureHandle_);
  }
}

// OpenGLDevice implementation
OpenGLDevice::OpenGLDevice()
  : window_(nullptr), isInitialized_(false) {
}

OpenGLDevice::~OpenGLDevice() {
  shutdown();
}

bool OpenGLDevice::initialize(Window* window) {
  try {
    window_ = window;

    if (!window_) {
      std::cerr << "Window required for graphics device initialization\n";
      return false;
    }

    window_->makeGLContextCurrent();

    // Initialize GLAD
    if (!initializeGLAD()) {
      std::cerr << "Failed to initialize GLAD\n";
      return false;
    }

    // Set OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, window_->getWidth(), window_->getHeight());

    isInitialized_ = true;
    std::cout << "OpenGL device initialized\n";
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    return true;
  } catch (const std::exception& e) {
    std::cerr << "OpenGL initialization error: " << e.what() << "\n";
    return false;
  }
}

void OpenGLDevice::shutdown() {
  isInitialized_ = false;
}

bool OpenGLDevice::initializeGLAD() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to load OpenGL functions with GLAD\n";
    return false;
  }
  return true;
}

Shader* OpenGLDevice::createShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
  auto shader = new OpenGLShader(vertexSrc, fragmentSrc);

  if (!shader->isValid()) {
    std::cerr << "Shader creation failed: " << shader->getErrorLog() << "\n";
    delete shader;
    return nullptr;
  }

  return shader;
}

void OpenGLDevice::deleteShader(Shader* shader) {
  delete shader;
}

Texture* OpenGLDevice::createTexture(int width, int height, const void* data) {
  return new OpenGLTexture(width, height, data);
}

void OpenGLDevice::deleteTexture(Texture* texture) {
  delete texture;
}

void OpenGLDevice::clear(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLDevice::executeRenderCommand(const RenderCommand& cmd) {
  // TODO: Implement render command execution
  // This will draw geometry with the specified shader
}

void OpenGLDevice::present() {
  if (window_) {
    window_->swapBuffers();
  }
}
