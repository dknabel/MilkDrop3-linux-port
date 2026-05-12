// src/platform/window.h
#pragma once
#include <string>
#include "types.h"

class Window {
public:
  virtual ~Window() = default;

  virtual bool create(int width, int height, bool fullscreen) = 0;
  virtual void shutdown() = 0;

  virtual bool update() = 0;
  virtual bool shouldClose() = 0;

  virtual void getInputState(InputState& outState) = 0;
  virtual void makeGLContextCurrent() = 0;
  virtual void swapBuffers() = 0;

  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
};

// Factory function
Window* createWindow();
void deleteWindow(Window* window);
