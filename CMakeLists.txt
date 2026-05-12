cmake_minimum_required(VERSION 3.16)
project(Milkdrop3 CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CMP0135 for FetchContent
cmake_policy(SET CMP0135 NEW)

message(STATUS "=== Milkdrop3 CMake Configuration ===")

# Find basic dependencies
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OPENGL_FOUND}")

find_package(PkgConfig REQUIRED)
message(STATUS "PkgConfig found")

# KissFFT for audio analysis - PRIMARY FOCUS FOR PHASE 4.1
message(STATUS "")
message(STATUS "=== KissFFT Integration ===")
include(FetchContent)

# KissFFT fetch with patch to add CMakeLists.txt
FetchContent_Declare(
  kissfft
  URL https://github.com/mborgerding/kissfft/archive/refs/tags/v131.tar.gz
  URL_HASH SHA256=a4f28cac6da18a5af7726a442416de87a6a3108fa12a59e101fa2f23defe4cf7
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/kissfft-CMakeLists.txt"
    "<SOURCE_DIR>/CMakeLists.txt"
)

message(STATUS "Fetching KissFFT from GitHub...")
FetchContent_MakeAvailable(kissfft)
message(STATUS "KissFFT fetched successfully")
message(STATUS "KissFFT source dir: ${kissfft_SOURCE_DIR}")
message(STATUS "KissFFT binary dir: ${kissfft_BINARY_DIR}")

# Verify KissFFT target
message(STATUS "Verifying KissFFT target...")
if(TARGET kissfft::kissfft)
  message(STATUS "SUCCESS: kissfft::kissfft target found")
  get_target_property(KISSFFT_INCLUDE_DIRS kissfft::kissfft INTERFACE_INCLUDE_DIRECTORIES)
  message(STATUS "KissFFT include dirs: ${KISSFFT_INCLUDE_DIRS}")
  message(STATUS "KissFFT headers available:")
  if(EXISTS "${kissfft_SOURCE_DIR}/kiss_fft.h")
    message(STATUS "  - ${kissfft_SOURCE_DIR}/kiss_fft.h [OK]")
  endif()
  if(EXISTS "${kissfft_SOURCE_DIR}/kissfft.hh")
    message(STATUS "  - ${kissfft_SOURCE_DIR}/kissfft.hh [OK]")
  endif()
  if(EXISTS "${kissfft_SOURCE_DIR}/kissfft_i32.hh")
    message(STATUS "  - ${kissfft_SOURCE_DIR}/kissfft_i32.hh [OK]")
  endif()
else()
  message(FATAL_ERROR "ERROR: kissfft::kissfft target NOT found")
endif()

# Audio backend detection
message(STATUS "")
message(STATUS "=== Audio Backend Detection ===")
pkg_check_modules(PIPEWIRE libpipewire-0.3)
pkg_check_modules(PULSEAUDIO libpulse)

if(PIPEWIRE_FOUND)
  message(STATUS "PipeWire found: ${PIPEWIRE_VERSION}")
else()
  message(STATUS "PipeWire NOT found")
endif()

if(PULSEAUDIO_FOUND)
  message(STATUS "PulseAudio found: ${PULSEAUDIO_VERSION}")
else()
  message(STATUS "PulseAudio NOT found")
endif()

if(NOT PIPEWIRE_FOUND AND NOT PULSEAUDIO_FOUND)
  message(FATAL_ERROR "Neither PipeWire nor PulseAudio development files found")
endif()

message(STATUS "")
message(STATUS "=== Phase 4.1 Task 4: CMake Configuration Complete ===")
message(STATUS "Status: SUCCESS")
message(STATUS "KissFFT integration verified and ready")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")
message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
