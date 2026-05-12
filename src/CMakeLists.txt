# Build core libraries first (expression, preset, etc)
add_subdirectory(core)

set(MILKDROP_SOURCES
  main.cpp
  platform/config.cpp
  platform/types.h
  audio/audio_analyzer.cpp
  audio/factory.cpp
  window/input.h
  ui/preset_manager.cpp
)

# Platform-specific sources (disabled for now - will be fixed in later tasks)
# if(PIPEWIRE_FOUND)
#   list(APPEND MILKDROP_SOURCES audio/pipewire_input.cpp)
# endif()

# if(PULSEAUDIO_FOUND)
#   list(APPEND MILKDROP_SOURCES audio/pulseaudio_input.cpp)
# endif()

list(APPEND MILKDROP_SOURCES
  window/factory.cpp
  graphics/factory.cpp
  core/visualization.cpp
)

add_executable(milkdrop3 ${MILKDROP_SOURCES})

# Link core libraries (preset parser, expression evaluator, etc)
target_link_libraries(milkdrop3 PRIVATE
  preset_parser
  expression_evaluator
  eel2
)

# Link KissFFT
target_link_libraries(milkdrop3 PRIVATE kissfft::kissfft)
target_include_directories(milkdrop3 PRIVATE ${kissfft_SOURCE_DIR})

target_include_directories(milkdrop3 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

# target_link_libraries(milkdrop3
#   # glfw (will be added back when window dependencies are resolved)
# )

# Audio backend linking (disabled for now - will be fixed in later tasks)
# if(PIPEWIRE_FOUND)
#   target_link_libraries(milkdrop3 ${PIPEWIRE_LIBRARIES})
#   target_include_directories(milkdrop3 PRIVATE ${PIPEWIRE_INCLUDE_DIRS})
#   target_compile_definitions(milkdrop3 PRIVATE HAVE_PIPEWIRE)
# endif()

# if(PULSEAUDIO_FOUND)
#   target_link_libraries(milkdrop3 ${PULSEAUDIO_LIBRARIES})
#   target_include_directories(milkdrop3 PRIVATE ${PULSEAUDIO_INCLUDE_DIRS})
#   target_compile_definitions(milkdrop3 PRIVATE HAVE_PULSEAUDIO)
# endif()
