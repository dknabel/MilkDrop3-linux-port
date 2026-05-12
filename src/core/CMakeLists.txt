# Core libraries
add_subdirectory(expression)
add_subdirectory(preset)

add_library(profiler STATIC
  profiler.cpp
)

target_include_directories(profiler PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
