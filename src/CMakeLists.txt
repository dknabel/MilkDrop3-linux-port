set(MILKDROP_SOURCES
  glad.c
  main.cpp
  platform/config.cpp
  platform/types.h
  audio/audio_analyzer.cpp
  audio/factory.cpp
  graphics/shader.cpp
  graphics/opengl_device.cpp
  graphics/factory.cpp
  window/input.h
  ui/preset_manager.cpp
  ui/overlay.cpp
)

# Platform-specific sources
if(PIPEWIRE_FOUND)
  list(APPEND MILKDROP_SOURCES audio/pipewire_input.cpp)
endif()

if(PULSEAUDIO_FOUND)
  list(APPEND MILKDROP_SOURCES audio/pulseaudio_input.cpp)
endif()

list(APPEND MILKDROP_SOURCES
  window/glfw_window.cpp
  window/factory.cpp
  graphics/opengl_device.cpp
  core/visualization.cpp
)

add_executable(milkdrop3 ${MILKDROP_SOURCES})

target_include_directories(milkdrop3 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(milkdrop3
  OpenGL::OpenGL
  glfw
  glm::glm
)

if(PIPEWIRE_FOUND)
  target_link_libraries(milkdrop3 ${PIPEWIRE_LIBRARIES})
  target_include_directories(milkdrop3 PRIVATE ${PIPEWIRE_INCLUDE_DIRS})
  target_compile_definitions(milkdrop3 PRIVATE HAVE_PIPEWIRE)
endif()

if(PULSEAUDIO_FOUND)
  target_link_libraries(milkdrop3 ${PULSEAUDIO_LIBRARIES})
  target_include_directories(milkdrop3 PRIVATE ${PULSEAUDIO_INCLUDE_DIRS})
  target_compile_definitions(milkdrop3 PRIVATE HAVE_PULSEAUDIO)
endif()
