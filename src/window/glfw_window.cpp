// src/window/glfw_window.cpp
#include "glfw_window.h"
#include "input.h"
#include <iostream>
#include <stdexcept>

// Global error callback
void glfwErrorCallback(int error, const char* description) {
  std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

GLFWWindow::GLFWWindow()
  : window_(nullptr), width_(0), height_(0), isInitialized_(false) {
  glfwSetErrorCallback(glfwErrorCallback);
}

GLFWWindow::~GLFWWindow() {
  shutdown();
}

bool GLFWWindow::create(int width, int height, bool fullscreen) {
  try {
    width_ = width;
    height_ = height;

    if (!glfwInit()) {
      std::cerr << "Failed to initialize GLFW\n";
      return false;
    }

    // Request OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    // Create window
    GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    window_ = glfwCreateWindow(width, height, "Milkdrop3", monitor, nullptr);

    if (!window_) {
      std::cerr << "Failed to create GLFW window\n";
      glfwTerminate();
      return false;
    }

    makeGLContextCurrent();

    // Enable V-Sync
    glfwSwapInterval(1);

    isInitialized_ = true;
    std::cout << "GLFW window created (" << width << "x" << height << ")\n";
    return true;
  } catch (const std::exception& e) {
    std::cerr << "GLFW initialization error: " << e.what() << "\n";
    return false;
  }
}

void GLFWWindow::shutdown() {
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }

  glfwTerminate();
  isInitialized_ = false;
}

bool GLFWWindow::update() {
  if (!isInitialized_) return false;
  glfwPollEvents();
  return true;
}

bool GLFWWindow::shouldClose() {
  if (!isInitialized_) return true;
  return glfwWindowShouldClose(window_);
}

void GLFWWindow::getInputState(InputState& outState) {
  outState = InputHandler::getInputState(window_);
}

void GLFWWindow::makeGLContextCurrent() {
  if (window_) {
    glfwMakeContextCurrent(window_);
  }
}

void GLFWWindow::swapBuffers() {
  if (window_) {
    glfwSwapBuffers(window_);
  }
}

// Input handler implementation
InputState InputHandler::getInputState(GLFWwindow* window) {
  InputState state = {};

  if (!window) return state;

  state.key_escape = isKeyPressed(window, GLFW_KEY_ESCAPE);
  state.key_p = isKeyPressed(window, GLFW_KEY_P);
  state.key_f = isKeyPressed(window, GLFW_KEY_F);
  state.key_s = isKeyPressed(window, GLFW_KEY_S);
  state.key_left = isKeyPressed(window, GLFW_KEY_LEFT);
  state.key_right = isKeyPressed(window, GLFW_KEY_RIGHT);
  state.key_up = isKeyPressed(window, GLFW_KEY_UP);
  state.key_down = isKeyPressed(window, GLFW_KEY_DOWN);

  return state;
}

bool InputHandler::isKeyPressed(GLFWwindow* window, int glfwKey) {
  return glfwGetKey(window, glfwKey) == GLFW_PRESS;
}
