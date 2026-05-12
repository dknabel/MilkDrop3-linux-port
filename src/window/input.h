// src/window/input.h
#pragma once
#include "../platform/types.h"
#include <GLFW/glfw3.h>

class InputHandler {
public:
  static InputState getInputState(GLFWwindow* window);

private:
  static bool isKeyPressed(GLFWwindow* window, int glfwKey);
};
