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

# KissFFT for audio analysis
include(FetchContent)
FetchContent_Declare(
  kissfft
  URL https://github.com/mborgerding/kissfft/archive/refs/tags/v131.tar.gz
  URL_HASH SHA256=da0ce21161e4df1677da79654e5599abe5ceee1f373c3b434c22e79ad0a87e5a
)
FetchContent_MakeAvailable(kissfft)

# GLAD loader (header-only version)
# Download from https://glad.dav1d.de/ if not present
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/include/glad/glad.h")
  message(WARNING "GLAD not found. Download from https://glad.dav1d.de/:
  1. Select OpenGL version 3.3 (Core profile)
  2. Extract to include/ directory
  Continuing without GLAD - build will fail at link time")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# Subdirectory
add_subdirectory(src)

# Link KissFFT to milkdrop3 target (after src subdirectory)
target_link_libraries(milkdrop3 kissfft::kissfft)
target_include_directories(milkdrop3 PRIVATE ${kissfft_SOURCE_DIR})

# Install targets
install(TARGETS milkdrop3 RUNTIME DESTINATION bin)
