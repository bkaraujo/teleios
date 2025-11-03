# TELEIOS Game Engine

A cross-platform game engine written in C11 with modern build system and automatic dependency management.

## Features

- **Cross-Platform**: Windows, Linux, and macOS support
- **Modern Build System**: CMake-based with automatic dependency management
- **Platform Abstraction**: Clean abstraction layer for OS-specific functionality
- **Automatic GLFW Integration**: Window and input handling library downloaded automatically
- **Modular Architecture**: Separate engine library and sandbox application
- **Debug & Release Builds**: Optimized configurations for development and production

## Quick Start

### 1. Install Prerequisites

See [SETUP.md](SETUP.md) for detailed installation instructions.

You need:
- CMake 3.20+
- C11 compiler (MSVC, GCC, or Clang)
- Git

### 2. Build the Project

```powershell
# Windows
.\build.ps1

# Or use CMake directly
mkdir build && cd build
cmake ..
cmake --build .
```

### 3. Run the Applications

```powershell
# Run the engine
.\build\bin\Debug\engine.exe   # Windows
./build/bin/engine             # Linux/macOS

# Run the sandbox
.\build\bin\Debug\sandbox.exe  # Windows
./build/bin/sandbox            # Linux/macOS
```

## Project Structure

```
TELEIOS/
├── engine/              # Core engine library
│   ├── src/main/
│   │   └── teleios/    # Engine modules
│   │       ├── platform.h/c      # Platform abstraction
│   │       ├── logger.h/c        # Logging system
│   │       ├── application.h/c   # Application framework
│   │       └── defines.h         # Platform detection & types
│   └── CMakeLists.txt
│
├── sandbox/            # Example application/game
│   ├── src/main/
│   └── CMakeLists.txt
│
├── build/              # Build output (generated)
│   ├── bin/           # Executables (engine.exe, sandbox.exe)
│   ├── lib/           # Static libraries (glfw3.lib)
│   └── _deps/         # Downloaded dependencies (GLFW)
│
├── CMakeLists.txt     # Root CMake configuration
├── build.ps1          # Convenient build script
├── SETUP.md           # Detailed setup instructions
├── CMAKE_BUILD.md     # CMake build guide
└── CLAUDE.md          # Development documentation

```

## Build Options

```powershell
# Debug build (default)
.\build.ps1

# Release build (optimized)
.\build.ps1 -Release

# Clean build
.\build.ps1 -Clean

# Build only engine
.\build.ps1 -EngineOnly

# Build only sandbox
.\build.ps1 -SandboxOnly

# Specify generator
.\build.ps1 -Generator "Visual Studio 17 2022"
```

## Documentation

- **[SETUP.md](SETUP.md)** - First-time setup and prerequisites
- **[CMAKE_BUILD.md](CMAKE_BUILD.md)** - Build system details and advanced options
- **[CLAUDE.md](CLAUDE.md)** - Architecture, conventions, and development workflow

## Architecture Overview

### Engine Modules

- **Platform Abstraction** ([platform.h](engine/src/main/teleios/platform.h))
  - OS detection and initialization
  - Platform-specific implementations for Windows, Linux, macOS

- **Logging System** ([logger.h](engine/src/main/teleios/logger.h))
  - Multiple log levels (VERBOSE, TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
  - Debug logs compiled out in release builds

- **Application Framework** ([application.h](engine/src/main/teleios/application.h))
  - Main game loop
  - Initialization and cleanup

- **Type System** ([defines.h](engine/src/main/teleios/defines.h))
  - Custom type aliases (`u8`, `u16`, `u32`, `u64`, `f32`, `f64`, etc.)
  - Platform detection macros
  - API visibility controls

### Dependencies

- **GLFW** - Automatically downloaded and built by CMake
  - Window creation and management
  - Input handling
  - OpenGL context creation

## Naming Conventions

- **Build-time preprocessors**: `TELEIOS_*` (e.g., `TELEIOS_BUILD_DEBUG`)
- **Runtime macros/constants**: `TL_<NAME>` (e.g., `TL_PLATFORM_WINDOWS`)
- **Macro functions**: `TL<Name>` (e.g., `TLINFO()`, `TLERROR()`)
- **API functions**: `tl_module_function()` (e.g., `tl_platform_initialize()`)
- **Type aliases**: lowercase (e.g., `u8`, `b8`, `f32`)

## Development

### Adding New Modules

1. Create header and implementation in `engine/src/main/teleios/`
2. Add to `engine/CMakeLists.txt` sources list
3. Include in `teleios.h` if it's a core module
4. Use `TL_API` macro for exported functions

### Platform-Specific Code

1. Add detection in `defines.h` if needed
2. Implement in `platform_<os>.c` files
3. Use `#ifdef TL_PLATFORM_*` guards
4. Provide fallback in `platform_agnostic.c` when applicable

## Compiler Support

- **MSVC** - Visual Studio 2019+
- **GCC** - GCC 7.0+
- **Clang** - Clang 6.0+

All compilers must support C11 standard.

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]

## Support

For issues and questions:
- Check [SETUP.md](SETUP.md) for installation problems
- Review [CMAKE_BUILD.md](CMAKE_BUILD.md) for build issues
- See [CLAUDE.md](CLAUDE.md) for development questions
