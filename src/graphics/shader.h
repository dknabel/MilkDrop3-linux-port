// src/graphics/shader.h
#pragma once
#include "../platform/graphics.h"
#include <string>
#include <glm/glm.hpp>

class OpenGLShader : public Shader {
public:
  OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
  ~OpenGLShader();

  uint32_t getHandle() const override { return programHandle_; }
  bool isValid() const override { return isValid_; }
  std::string getErrorLog() const override { return errorLog_; }

  void use() const;
  void setUniform(const std::string& name, float value) const;
  void setUniform(const std::string& name, const glm::vec3& value) const;
  void setUniform(const std::string& name, const glm::mat4& value) const;

private:
  uint32_t programHandle_;
  bool isValid_;
  std::string errorLog_;

  bool compile(const std::string& vertexSrc, const std::string& fragmentSrc);
  uint32_t compileShaderStage(const std::string& source, uint32_t stage);
};
