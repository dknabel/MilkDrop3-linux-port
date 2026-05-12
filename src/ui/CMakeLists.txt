# UI Components Library
add_library(ui STATIC
  display_manager.cpp
)

# Include directories
target_include_directories(ui PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
  ${GLFW_INCLUDE_DIR}
)

target_link_libraries(ui PUBLIC
  glfw
  OpenGL::OpenGL
)
