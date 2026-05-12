# Stability and Crash Resilience Tests
add_executable(test_crash_resilience
  test_crash_resilience.cpp
  ${CMAKE_SOURCE_DIR}/src/audio/audio_analyzer.cpp
  ${CMAKE_SOURCE_DIR}/src/platform/types.h
)

target_include_directories(test_crash_resilience PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
  ${GLFW_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/build/_deps/glad-src/include
  ${PROJECT_SOURCE_DIR}/build/_deps/glm-src
  ${kissfft_SOURCE_DIR}
)

target_link_libraries(test_crash_resilience
  gtest_main
  gtest
  ui
  visualization
  preset_parser
  kissfft::kissfft
  glfw
  OpenGL::OpenGL
  glad
)

add_test(NAME CrashResilience COMMAND test_crash_resilience)
