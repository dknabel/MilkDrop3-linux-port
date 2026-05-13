// src/graphics/opengl_device.cpp
#include "opengl_device.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include <vector>

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
  : window_(nullptr), isInitialized_(false), defaultShader_(nullptr) {
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
  if (waveformVAO_) {
    glDeleteVertexArrays(1, &waveformVAO_);
    waveformVAO_ = 0;
  }
  if (waveformVBO_) {
    glDeleteBuffers(1, &waveformVBO_);
    waveformVBO_ = 0;
  }
  if (shapeVAO_) {
    glDeleteVertexArrays(1, &shapeVAO_);
    shapeVAO_ = 0;
  }
  if (shapeVBO_) {
    glDeleteBuffers(1, &shapeVBO_);
    shapeVBO_ = 0;
  }
  if (defaultShader_) {
    deleteShader(defaultShader_);
    defaultShader_ = nullptr;
  }
  isInitialized_ = false;
}

bool OpenGLDevice::initializeGLAD() {
  // Initialize GLEW instead of GLAD
  glewExperimental = GL_TRUE;
  GLenum glewErr = glewInit();
  if (glewErr != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << "\n";
    return false;
  }
  std::cout << "GLEW initialized successfully\n";
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
  if (!isInitialized_) return;

  switch (cmd.type) {
    case RenderCommandType::Clear:
      executeClearCommand(cmd);
      break;
    case RenderCommandType::DrawWaveform:
      executeDrawWaveformCommand(cmd);
      break;
    case RenderCommandType::DrawShape:
      executeDrawShapeCommand(cmd);
      break;
    default:
      break;
  }
}

void OpenGLDevice::executeClearCommand(const RenderCommand& cmd) {
  glClearColor(cmd.clearColor[0], cmd.clearColor[1],
               cmd.clearColor[2], cmd.clearColor[3]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLDevice::executeDrawWaveformCommand(const RenderCommand& cmd) {
  if (cmd.frequencyBins.empty()) return;

  // Create default shader if not already created
  if (!defaultShader_) {
    std::string vertexShader(DefaultShaderSource::vertexShader);
    std::string fragmentShader(DefaultShaderSource::fragmentShader);
    defaultShader_ = createShader(vertexShader, fragmentShader);
    if (!defaultShader_) {
      std::cerr << "Failed to create default shader for waveform rendering\n";
      return;
    }
  }

  OpenGLShader* shader = dynamic_cast<OpenGLShader*>(defaultShader_);
  if (!shader) return;

  shader->use();

  // Set uniforms
  glm::mat4 projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 model = glm::mat4(1.0f);

  shader->setUniform("projection", projection);
  shader->setUniform("view", view);
  shader->setUniform("model", model);
  shader->setUniform("color", glm::vec4(cmd.waveColor[0], cmd.waveColor[1],
                                        cmd.waveColor[2], cmd.waveColor[3]));

  // Build vertex data from frequency bins
  std::vector<glm::vec2> vertices;
  for (size_t i = 0; i < cmd.frequencyBins.size(); ++i) {
    float x = static_cast<float>(i) / cmd.frequencyBins.size();
    float y = cmd.frequencyBins[i];
    vertices.push_back({x, y});
  }

  // Create or update VAO/VBO
  if (waveformVAO_ == 0) {
    glGenVertexArrays(1, &waveformVAO_);
    glGenBuffers(1, &waveformVBO_);
  }

  glBindVertexArray(waveformVAO_);
  glBindBuffer(GL_ARRAY_BUFFER, waveformVBO_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_DYNAMIC_DRAW);

  // Set vertex attributes
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

  // Draw
  glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(vertices.size()));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  waveformVertexCount_ = vertices.size();
}

void OpenGLDevice::executeDrawShapeCommand(const RenderCommand& cmd) {
  // Create default shader if not already created
  if (!defaultShader_) {
    std::string vertexShader(DefaultShaderSource::vertexShader);
    std::string fragmentShader(DefaultShaderSource::fragmentShader);
    defaultShader_ = createShader(vertexShader, fragmentShader);
    if (!defaultShader_) {
      std::cerr << "Failed to create default shader for shape rendering\n";
      return;
    }
  }

  OpenGLShader* shader = dynamic_cast<OpenGLShader*>(defaultShader_);
  if (!shader) return;

  shader->use();

  // Set blending mode
  if (cmd.shapeAdditive) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  } else {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  // Set uniforms
  glm::mat4 projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
  glm::mat4 view = glm::mat4(1.0f);

  // Build model matrix from position and radius
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(cmd.shapePosition[0], cmd.shapePosition[1], 0.0f));
  model = glm::rotate(model, cmd.shapeAngle, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(cmd.shapeRadius, cmd.shapeRadius, 1.0f));

  shader->setUniform("projection", projection);
  shader->setUniform("view", view);
  shader->setUniform("model", model);
  shader->setUniform("color", glm::vec4(cmd.shapeColor[0], cmd.shapeColor[1],
                                        cmd.shapeColor[2], cmd.shapeColor[3]));

  // Generate polygon vertices based on number of sides
  int sides = cmd.shapeSides;
  if (sides < 3) sides = 3;
  if (sides > 256) sides = 256;

  std::vector<glm::vec2> vertices;
  const float PI = 3.14159265f;
  for (int i = 0; i < sides; ++i) {
    float angle = (2.0f * PI * i) / sides;
    vertices.push_back(glm::vec2(std::cos(angle) * 0.5f, std::sin(angle) * 0.5f));
  }

  if (shapeVAO_ == 0) {
    glGenVertexArrays(1, &shapeVAO_);
    glGenBuffers(1, &shapeVBO_);
  }

  glBindVertexArray(shapeVAO_);
  glBindBuffer(GL_ARRAY_BUFFER, shapeVBO_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

  glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(vertices.size()));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Restore default blending
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLDevice::present() {
  if (window_) {
    window_->swapBuffers();
  }
}
