#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "../platform/types.h"

class DisplayManager {
public:
  DisplayManager();
  ~DisplayManager();

  // Initialize GLFW window and OpenGL context
  bool initialize(int width = 1280, int height = 720, const std::string& title = "Milkdrop3");

  // Process events and update window state
  bool update();

  // Render a frame from render commands
  void render(const std::vector<RenderCommand>& commands);

  // Cleanup and shutdown
  void shutdown();

  // Query state
  bool isRunning() const { return !glfwWindowShouldClose(window_); }
  glm::ivec2 getWindowSize() const { return windowSize_; }
  float getAspectRatio() const { return static_cast<float>(windowSize_.x) / windowSize_.y; }
  double getElapsedTime() const;

  // Input handling
  bool isKeyPressed(int key) const;

private:
  GLFWwindow* window_;
  glm::ivec2 windowSize_;
  double lastFrameTime_;

  void setupOpenGL();
  void handleInput();
};
