# TELEIOS Setup Guide

This guide will help you set up your development environment for building TELEIOS.

## Prerequisites

### 1. CMake

CMake is required to configure and build the project.

**Installation:**
- **Windows (winget)**: `winget install Kitware.CMake`
- **Windows (chocolatey)**: `choco install cmake`
- **Windows (manual)**: Download from https://cmake.org/download/
- **Linux**: `sudo apt install cmake` (Debian/Ubuntu) or `sudo dnf install cmake` (Fedora)
- **macOS**: `brew install cmake`

**Verify installation:**
```bash
cmake --version
```
Required: CMake 3.20 or higher

### 2. Git

Git is required for downloading dependencies (GLFW).

**Installation:**
- **Windows**: Download from https://git-scm.com/downloads
- **Linux**: `sudo apt install git` or `sudo dnf install git`
- **macOS**: Included with Xcode Command Line Tools (`xcode-select --install`)

**Verify installation:**
```bash
git --version
```

### 3. C Compiler and Build Tools

Choose ONE of the following options based on your platform and preference:

## Windows Options

### Option 1: Visual Studio (RECOMMENDED)

**Best for:** Windows developers, integrated debugging, modern toolchain

**Installation:**
1. Download Visual Studio Community (free): https://visualstudio.microsoft.com/downloads/
2. During installation, select "Desktop development with C++"
3. This includes:
   - MSVC compiler
   - Windows SDK
   - CMake integration
   - MSBuild

**Verify:**
```powershell
# Open "Developer Command Prompt for VS" or "Developer PowerShell for VS"
cl
nmake
```

### Option 2: MinGW-w64 via MSYS2

**Best for:** GCC compatibility, open-source toolchain, smaller install size

**Installation:**
1. Download MSYS2: https://www.msys2.org/
2. Install MSYS2 to `C:\msys64` (default)
3. Open "MSYS2 MINGW64" terminal
4. Update package database:
   ```bash
   pacman -Syu
   ```
5. Install MinGW-w64 toolchain:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
   ```
6. Add to Windows PATH:
   - Open "Environment Variables" in Windows settings
   - Add `C:\msys64\mingw64\bin` to your PATH
   - Restart PowerShell/Terminal

**Verify:**
```bash
gcc --version
mingw32-make --version
```

### Option 3: Clang/LLVM

**Best for:** Cross-platform compatibility, modern C features, alternative to MSVC

**Installation:**
1. Download LLVM: https://releases.llvm.org/download.html
2. During installation, check "Add LLVM to system PATH"
3. Install Ninja build system (optional but recommended):
   ```bash
   winget install Ninja-build.Ninja
   ```

**Verify:**
```bash
clang --version
ninja --version
```

## Linux Setup

### Debian/Ubuntu

```bash
# Install everything in one command
sudo apt update
sudo apt install build-essential cmake git libgl1-mesa-dev

# Verify
gcc --version
cmake --version
git --version
```

### Fedora

```bash
# Install everything in one command
sudo dnf install gcc gcc-c++ cmake git mesa-libGL-devel

# Verify
gcc --version
cmake --version
git --version
```

### Arch Linux

```bash
# Install everything in one command
sudo pacman -S base-devel cmake git mesa

# Verify
gcc --version
cmake --version
git --version
```

## macOS Setup

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake
brew install cmake

# Verify
clang --version
cmake --version
git --version
```

## Building TELEIOS

Once you have all prerequisites installed:

### Quick Start

```powershell
# Windows PowerShell
.\build.ps1

# The script will automatically detect your compiler and build the project
```

### Build Options

```powershell
# Debug build (default)
.\build.ps1

# Release build (optimized)
.\build.ps1 -Release

# Clean build (remove old files first)
.\build.ps1 -Clean

# Build only the engine
.\build.ps1 -EngineOnly

# Build only the sandbox
.\build.ps1 -SandboxOnly

# Specify a generator manually
.\build.ps1 -Generator "Visual Studio 17 2022"
.\build.ps1 -Generator "MinGW Makefiles"
.\build.ps1 -Generator "Ninja"
```

### Manual CMake Build

If you prefer to use CMake directly:

```bash
# Create and enter build directory
mkdir build
cd build

# Configure (choose your generator)
cmake -G "MinGW Makefiles" ..           # Windows with MinGW
cmake -G "Visual Studio 17 2022" ..     # Windows with Visual Studio
cmake -G "Unix Makefiles" ..            # Linux/macOS
cmake -G "Ninja" ..                     # Any platform with Ninja

# Build
cmake --build . --config Debug

# Or build specific targets
cmake --build . --target engine --config Debug
cmake --build . --target sandbox --config Debug
```

## Troubleshooting

### "CMake not found"
- Ensure CMake is installed and in your PATH
- Restart your terminal/PowerShell after installation
- Run `cmake --version` to verify

### "No suitable build tools found"
- Install a compiler (see options above)
- Ensure the compiler is in your PATH
- For Visual Studio: Use "Developer PowerShell for VS"
- For MinGW: Ensure `C:\msys64\mingw64\bin` is in PATH

### "Git not found" during GLFW download
- Install Git: https://git-scm.com/downloads
- Ensure Git is in your PATH
- Restart terminal after installation

### "Cannot find -lopengl32" or OpenGL errors
- **Windows**: Install GPU drivers, or install Windows SDK
- **Linux**: Install OpenGL dev packages: `sudo apt install libgl1-mesa-dev`
- **macOS**: Ensure Xcode Command Line Tools are installed

### Build fails after changing compilers
- Run a clean build: `.\build.ps1 -Clean`
- Delete the `build` directory manually if needed

### "Permission denied" or "Access denied"
- Close any applications using the build output (debuggers, file explorers)
- Run `.\build.ps1 -Clean` to remove locked files
- On Windows: Check if antivirus is blocking the build

## IDE Setup

### Visual Studio Code

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)
   - CMake (twxs)

2. Open the TELEIOS folder in VS Code

3. CMake Tools will automatically detect the configuration

4. Use the CMake sidebar to configure and build

### Visual Studio

1. Open Visual Studio
2. Select "Open a local folder"
3. Navigate to the TELEIOS directory
4. Visual Studio will automatically detect CMakeLists.txt
5. Use the build toolbar to compile

### CLion

1. Open the TELEIOS directory
2. CLion automatically detects CMake projects
3. Configure CMake settings in Settings → Build → CMake
4. Use the build toolbar

## Next Steps

After successful build:
- Executables are in `build/bin/`
- Run the sandbox: `.\build\bin\sandbox.exe` (Windows) or `./build/bin/sandbox` (Linux/macOS)
- See [CMAKE_BUILD.md](CMAKE_BUILD.md) for advanced build options
- See [CLAUDE.md](CLAUDE.md) for architecture and development workflow
