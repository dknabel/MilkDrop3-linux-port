# Preset types library (header-only)
add_library(preset_types INTERFACE)
target_include_directories(preset_types INTERFACE
  ${CMAKE_SOURCE_DIR}/src
)

# Preset parser library
add_library(preset_parser STATIC
  preset_parser.cpp
)

target_include_directories(preset_parser PUBLIC
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(preset_parser PUBLIC
  preset_types
)
