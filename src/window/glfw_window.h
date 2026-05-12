// src/window/glfw_window.h
#pragma once
#include "../platform/window.h"
#include <GLFW/glfw3.h>

class GLFWWindow : public Window {
public:
  GLFWWindow();
  ~GLFWWindow();

  bool create(int width, int height, bool fullscreen) override;
  void shutdown() override;

  bool update() override;
  bool shouldClose() override;

  void getInputState(InputState& outState) override;
  void makeGLContextCurrent() override;
  void swapBuffers() override;

  int getWidth() const override { return width_; }
  int getHeight() const override { return height_; }

private:
  GLFWwindow* window_;
  int width_;
  int height_;
  bool isInitialized_;

  static void errorCallback(int error, const char* description);
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
