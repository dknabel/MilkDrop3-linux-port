# Core libraries
add_subdirectory(expression)
add_subdirectory(preset)

add_library(profiler STATIC
  profiler.cpp
)

target_include_directories(profiler PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

add_library(performance_baseline STATIC
  performance_baseline.cpp
)

target_include_directories(performance_baseline PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

add_library(visualization STATIC
  visualization.cpp
)

target_link_libraries(visualization PRIVATE
  preset_parser
  expression_evaluator
  profiler
)

target_include_directories(visualization PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
