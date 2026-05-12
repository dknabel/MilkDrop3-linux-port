# Building Milkdrop3

## Prerequisites

Before building Milkdrop3, ensure you have the following installed:

- **CMake** 3.16 or higher
- **C++17 compiler** (GCC 8+, Clang 5+, or equivalent)
- **GLFW3** development libraries
- **OpenGL** development libraries
- **PipeWire** or **PulseAudio** development libraries
- **GLM** header-only mathematics library

## Installing Dependencies

### Ubuntu/Debian

Run the following command to install all required packages:

```bash
sudo apt-get update
sudo apt-get install cmake g++ libglfw3-dev libgl1-mesa-dev \
  libpipewire-0.3-dev libpulse-dev libglm-dev
```

**Package breakdown:**
- `cmake` - Build system generator
- `g++` - C++ compiler
- `libglfw3-dev` - GLFW3 window and input library
- `libgl1-mesa-dev` - OpenGL development files
- `libpipewire-0.3-dev` - PipeWire audio library (optional but recommended)
- `libpulse-dev` - PulseAudio library (alternative audio backend)
- `libglm-dev` - GLM mathematics library

### Fedora/RHEL

```bash
sudo dnf install cmake gcc-c++ glfw-devel mesa-libGL-devel \
  pipewire-devel pulseaudio-devel glm-devel
```

### Arch Linux

```bash
sudo pacman -S cmake glfw-x11 mesa glm pipewire pulseaudio
```

## Setup GLAD Loader

GLAD is a loader library for OpenGL functions. You need to download and extract it before building.

### Step 1: Download GLAD

1. Visit https://glad.dav1d.de/
2. Configure the following settings:
   - **Language:** C/C++
   - **Specification:** OpenGL
   - **OpenGL Version:** 3.3 (Core profile)
   - Leave other options at defaults
3. Click **GENERATE** to create the loader
4. Download the generated `glad.zip` file

### Step 2: Extract GLAD to Project Root

Navigate to the project root directory and extract GLAD:

```bash
cd /path/to/molkdroop
unzip ~/Downloads/glad.zip
cp -r glad/include . 
```

This will create an `include/` directory in the project root containing the GLAD headers.

**Verify the setup:**
```bash
ls -la include/glad/glad.h  # Should exist and be readable
```

## Building

### Step 1: Create Build Directory

```bash
mkdir build
cd build
```

### Step 2: Configure with CMake

```bash
cmake ..
```

CMake will:
- Locate all dependencies
- Validate GLAD headers are present (warning if missing)
- Generate build files

### Step 3: Compile

```bash
make
```

This will compile all source files and produce the executable `milkdrop3`.

### Step 4: Run

Execute the application:

```bash
./milkdrop3
```

## Complete Build Script

To build from scratch, use this command sequence:

```bash
cd /path/to/molkdroop
mkdir -p build
cd build
cmake ..
make
./milkdrop3
```

## Troubleshooting

### CMake Cannot Find OpenGL/GLFW3

**Error:** `Could not find the following Cmake packages: OpenGL, GLFW3`

**Solution:** Ensure development libraries are installed:
```bash
sudo apt-get install libglfw3-dev libgl1-mesa-dev  # Ubuntu/Debian
```

### GLAD Warning During Build

**Warning:** `GLAD not found. Download from https://glad.dav1d.de/`

**Solution:** GLAD headers are missing. Follow the **Setup GLAD Loader** section above and re-run CMake.

### Compilation Errors with glad.h

**Error:** `fatal error: glad/glad.h: No such file or directory`

**Solution:** 
1. Verify GLAD extraction: `ls include/glad/glad.h`
2. Rebuild: `cd build && cmake .. && make clean && make`

### PipeWire/PulseAudio Not Found

**Warning:** `Neither PipeWire nor PulseAudio development files found`

**Solution:** Install at least one audio library:
```bash
sudo apt-get install libpipewire-0.3-dev  # For PipeWire
# OR
sudo apt-get install libpulse-dev         # For PulseAudio
```

### Link Errors

**Error:** `undefined reference to 'gladLoadGL'`

**Solution:**
1. Verify glad.c exists in src/: `ls src/glad.c`
2. Verify it's in src/CMakeLists.txt MILKDROP_SOURCES list
3. Rebuild: `cd build && cmake .. && make clean && make`

## Clean Build

To start fresh:

```bash
cd /path/to/molkdroop
rm -rf build
mkdir build
cd build
cmake ..
make
```

## Development

For development with verbose output:

```bash
cd build
cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..
make VERBOSE=1
```

For debug symbols:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## Installation

To install the compiled binary system-wide (optional):

```bash
cd build
sudo cmake --install .
```

This installs the `milkdrop3` executable to `/usr/local/bin/` or as configured.
