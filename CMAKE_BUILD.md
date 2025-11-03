# CMake Build System Guide

## Prerequisites

See [SETUP.md](SETUP.md) for detailed installation instructions.

Quick checklist:
- ✅ CMake 3.20 or higher
- ✅ C11-compatible compiler (MSVC, GCC, or Clang)
- ✅ Git (for downloading GLFW dependency)
- ✅ Build tools (Visual Studio, MinGW, or Make)

## Building the Project

### Windows (PowerShell)

```powershell
# Basic build (Debug mode)
.\build.ps1

# Clean build
.\build.ps1 -Clean

# Release build
.\build.ps1 -Release

# Build only engine
.\build.ps1 -EngineOnly

# Build only sandbox
.\build.ps1 -SandboxOnly

# Clean release build
.\build.ps1 -Clean -Release
```

### Manual CMake Build (All Platforms)

```bash
# Configure
mkdir build
cd build
cmake -G "MinGW Makefiles" ..  # Windows with MinGW
# OR
cmake -G "Unix Makefiles" ..    # Linux/macOS
# OR
cmake -G "Visual Studio 17 2022" ..  # Windows with MSVC

# Build
cmake --build . --config Debug

# Build specific target
cmake --build . --target engine --config Debug
cmake --build . --target sandbox --config Debug
```

## CMake Features

### Automatic GLFW Download

The build system automatically:
- Downloads GLFW 3.4 (latest stable) from GitHub
- Configures GLFW with optimal settings for TELEIOS
- Builds GLFW as part of the engine library
- Links everything together seamlessly

No manual dependency management required!

### Build Configurations

- **Debug**: Optimizations disabled (`-O0`), debug symbols enabled, `TELEIOS_BUILD_DEBUG` defined
- **Release**: Optimizations enabled (`-O2`/`-O3`), `TELEIOS_BUILD_RELEASE` defined

### Output Structure

```
build/
├── bin/              # Executables (sandbox.exe, engine.dll on Windows)
├── lib/              # Static/shared libraries
├── _deps/            # Downloaded dependencies (GLFW)
└── CMakeFiles/       # CMake internal files
```

## CMake Configuration Options

You can customize the build by passing options to CMake:

```bash
# Use a specific C compiler
cmake -DCMAKE_C_COMPILER=clang ..

# Specify build type
cmake -DCMAKE_BUILD_TYPE=Release ..

# Enable verbose build output
cmake --build . --verbose
```

## Platform-Specific Notes

### Windows

- The engine is built as a DLL (`engine.dll`)
- The DLL is automatically copied to the sandbox executable directory
- Recommended generators: MinGW Makefiles, Visual Studio, Ninja

### Linux

- The engine is built as a shared library (`libengine.so`)
- Requires OpenGL development libraries: `sudo apt install libgl1-mesa-dev`
- Recommended generator: Unix Makefiles or Ninja

### macOS

- The engine is built as a dylib (`libengine.dylib`)
- Requires Xcode Command Line Tools: `xcode-select --install`
- Recommended generator: Unix Makefiles or Xcode

## Troubleshooting

### CMake not found
- Ensure CMake is installed and in your PATH
- Restart your terminal after installing CMake

### GLFW download fails
- Check your internet connection
- Ensure Git is installed and accessible
- Check firewall/proxy settings

### Compiler not found
- Ensure you have a C11-compatible compiler installed
- Set `CMAKE_C_COMPILER` to point to your compiler

## Migration from PowerShell Build System

The old PowerShell build scripts (`build/compile.ps1`, `build_all.ps1`) are replaced by:
- CMake configuration files (`CMakeLists.txt`)
- New build script (`build.ps1`)

Benefits of CMake:
- ✅ Cross-platform build configuration
- ✅ Automatic dependency management (GLFW)
- ✅ Better IDE integration (Visual Studio, CLion, VS Code)
- ✅ Parallel compilation by default
- ✅ Industry-standard build system
- ✅ Easier to add new dependencies
