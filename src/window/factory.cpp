// src/window/factory.cpp
#include "../platform/window.h"
#include "glfw_window.h"

Window* createWindow() {
  return new GLFWWindow();
}

void deleteWindow(Window* window) {
  delete window;
}
