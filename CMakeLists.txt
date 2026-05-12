cmake_minimum_required(VERSION 3.16)
project(Milkdrop3 CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE libpipewire-0.3)
pkg_check_modules(PULSEAUDIO libpulse)

if(NOT PIPEWIRE_FOUND AND NOT PULSEAUDIO_FOUND)
  message(FATAL_ERROR "Neither PipeWire nor PulseAudio development files found")
endif()

# GLM header-only
find_package(GLM REQUIRED)

# Subdirectory
add_subdirectory(src)

# Install targets
install(TARGETS milkdrop3 RUNTIME DESTINATION bin)
