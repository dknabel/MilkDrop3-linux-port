// src/graphics/shader.cpp
#include "shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

namespace milkdrop {
constexpr size_t SHADER_ERROR_LOG_SIZE = 512;
} // namespace milkdrop

OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
  : programHandle_(0), isValid_(false) {
  compile(vertexSrc, fragmentSrc);
}

OpenGLShader::~OpenGLShader() {
  if (programHandle_) {
    glDeleteProgram(programHandle_);
  }
}

uint32_t OpenGLShader::compileShaderStage(const std::string& source, uint32_t stage) {
  uint32_t shader = glCreateShader(stage);
  const char* src = source.c_str();

  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    char infoLog[milkdrop::SHADER_ERROR_LOG_SIZE];
    glGetShaderInfoLog(shader, milkdrop::SHADER_ERROR_LOG_SIZE, nullptr, infoLog);
    errorLog_ = std::string(infoLog);
    std::cerr << "Shader compilation error:\n" << infoLog << "\n";
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

bool OpenGLShader::compile(const std::string& vertexSrc, const std::string& fragmentSrc) {
  uint32_t vertex = compileShaderStage(vertexSrc, GL_VERTEX_SHADER);
  if (!vertex) return false;

  uint32_t fragment = compileShaderStage(fragmentSrc, GL_FRAGMENT_SHADER);
  if (!fragment) {
    glDeleteShader(vertex);
    return false;
  }

  programHandle_ = glCreateProgram();
  glAttachShader(programHandle_, vertex);
  glAttachShader(programHandle_, fragment);
  glLinkProgram(programHandle_);

  int success;
  glGetProgramiv(programHandle_, GL_LINK_STATUS, &success);

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  if (!success) {
    char infoLog[milkdrop::SHADER_ERROR_LOG_SIZE];
    glGetProgramInfoLog(programHandle_, milkdrop::SHADER_ERROR_LOG_SIZE, nullptr, infoLog);
    errorLog_ = std::string(infoLog);
    std::cerr << "Shader linking error:\n" << infoLog << "\n";
    return false;
  }

  isValid_ = true;
  return true;
}

void OpenGLShader::use() const {
  glUseProgram(programHandle_);
}

void OpenGLShader::setUniform(const std::string& name, float value) const {
  int loc = glGetUniformLocation(programHandle_, name.c_str());
  if (loc != -1) {
    glUniform1f(loc, value);
  }
}

void OpenGLShader::setUniform(const std::string& name, const glm::vec3& value) const {
  int loc = glGetUniformLocation(programHandle_, name.c_str());
  if (loc != -1) {
    glUniform3fv(loc, 1, glm::value_ptr(value));
  }
}

void OpenGLShader::setUniform(const std::string& name, const glm::vec4& value) const {
  int loc = glGetUniformLocation(programHandle_, name.c_str());
  if (loc != -1) {
    glUniform4fv(loc, 1, glm::value_ptr(value));
  }
}

void OpenGLShader::setUniform(const std::string& name, const glm::mat4& value) const {
  int loc = glGetUniformLocation(programHandle_, name.c_str());
  if (loc != -1) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
  }
}
