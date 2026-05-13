#include "display_manager.h"
#include "../platform/graphics.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Note: GLAD is not directly used here; it's handled by the graphics device
// The OpenGL context is managed by GLFW and made current before rendering

static void glfwErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error (" << error << "): " << description << "\n";
}

DisplayManager::DisplayManager()
  : window_(nullptr), windowSize_(1280, 720), lastFrameTime_(0.0) {
}

DisplayManager::~DisplayManager() {
  shutdown();
}

bool DisplayManager::initialize(int width, int height, const std::string& title) {
  glfwSetErrorCallback(glfwErrorCallback);

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!window_) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return false;
  }

  windowSize_ = glm::ivec2(width, height);
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1); // Enable vsync

  setupOpenGL();
  lastFrameTime_ = glfwGetTime();

  std::cout << "Display manager initialized: " << width << "x" << height << "\n";
  return true;
}

void DisplayManager::setupOpenGL() {
  glViewport(0, 0, windowSize_.x, windowSize_.y);
  std::cout << "OpenGL viewport set to: " << windowSize_.x << "x" << windowSize_.y << "\n";
}

bool DisplayManager::update() {
  if (!window_ || glfwWindowShouldClose(window_)) {
    return false;
  }

  glfwPollEvents();
  handleInput();

  int newWidth, newHeight;
  glfwGetWindowSize(window_, &newWidth, &newHeight);
  if (newWidth != windowSize_.x || newHeight != windowSize_.y) {
    windowSize_ = glm::ivec2(newWidth, newHeight);
    glViewport(0, 0, windowSize_.x, windowSize_.y);
  }

  return true;
}

void DisplayManager::render(const std::vector<RenderCommand>& commands, GraphicsDevice* graphics) {
  if (!window_) return;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (graphics) {
    for (const auto& cmd : commands) {
      graphics->executeRenderCommand(cmd);
    }
  }

  glfwSwapBuffers(window_);
}

void DisplayManager::shutdown() {
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }
  glfwTerminate();
}

double DisplayManager::getElapsedTime() const {
  return glfwGetTime();
}

bool DisplayManager::isKeyPressed(int key) const {
  if (!window_) return false;
  return glfwGetKey(window_, key) == GLFW_PRESS;
}

void DisplayManager::handleInput() {
  if (isKeyPressed(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window_, true);
  }
}
