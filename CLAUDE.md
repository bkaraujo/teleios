# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TELEIOS is a cross-platform game engine written in C11. The project consists of two main components:
- **engine**: The core engine library with platform abstraction layers
- **sandbox**: Application/game that uses the engine

## Build System

The project uses **CMake** (version 3.20+) as its build system with automatic dependency management.

### Prerequisites

- CMake 3.20 or higher
- C11-compatible compiler (Clang, GCC, or MSVC)
- Git (for downloading dependencies)

See [CMAKE_BUILD.md](CMAKE_BUILD.md) for detailed setup instructions.

### Building the Project

```powershell
# Build everything (engine + sandbox) - Debug mode
.\build.ps1

# Clean build
.\build.ps1 -Clean

# Release build
.\build.ps1 -Release

# Build only engine
.\build.ps1 -EngineOnly

# Build only sandbox
.\build.ps1 -SandboxOnly
```

#### Manual CMake Build

```bash
# Configure
mkdir build && cd build
cmake -G "MinGW Makefiles" ..  # Windows with MinGW
# or
cmake -G "Unix Makefiles" ..    # Linux/macOS

# Build
cmake --build . --config Debug

# Build specific target
cmake --build . --target engine
cmake --build . --target sandbox
```

### Build System Architecture

The build system uses CMake with three configuration files:
1. **CMakeLists.txt** (root) - Main project configuration
2. **engine/CMakeLists.txt** - Engine library with GLFW integration
3. **sandbox/CMakeLists.txt** - Sandbox application configuration

### Automatic Dependency Management

All external libraries are automatically downloaded and built by CMake using FetchContent:

**GLFW 3.4** - Windowing and input handling
- Automatically downloaded from https://github.com/glfw/glfw.git
- Linked statically with both engine and sandbox

**libyaml 0.2.5** - YAML parser/emitter
- Automatically downloaded from https://github.com/yaml/libyaml.git
- Built manually as static library (bypasses CMake version issues)
- Linked statically with engine only

**Lua 5.4.7** - Embeddable scripting language
- Automatically downloaded from https://github.com/lua/lua.git
- Built as static library (excludes lua.c, luac.c, onelua.c)
- Linked statically with engine only

No manual dependency installation required - just run `.\build.ps1`

### Engine Build Configuration

The engine is built as a standalone executable with:
- **Defines**:
  - Debug: `TELEIOS_BUILD_DEBUG`
  - Release: `TELEIOS_BUILD_RELEASE`
- **Link libraries**:
  - Cross-platform: GLFW, libyaml (`yaml`), Lua (`lua_static`)
  - Windows: `user32`, `gdi32`, `kernel32`, `opengl32`
  - Linux: `m`, `pthread`, `dl`, OpenGL
  - macOS: Cocoa, IOKit, CoreVideo, OpenGL
- **Compile flags**:
  - MSVC: `/W4`, `/std:c11`, `/experimental:c11atomics`
  - GCC/Clang: `-Wall`, `-Wextra`, `-std=c11`
  - Debug: `-O0`/`/Od`, `-g`/`/Zi`
  - Release: `-O3`/`/O2`
- **Output**: `engine.exe` (Windows), `engine` (Linux/macOS)

### Sandbox Build Configuration

The sandbox is built as a standalone executable with:
- **Defines**:
  - Debug: `TELEIOS_BUILD_DEBUG`
  - Release: `TELEIOS_BUILD_RELEASE`
- **Link libraries**:
  - Cross-platform: GLFW (linked directly)
  - Windows: `user32`, `gdi32`, `kernel32`
  - Linux: `m`, `pthread`
- **Compile flags**:
  - MSVC: `/W4`, `/std:c11`, `/experimental:c11atomics`
  - GCC/Clang: `-Wall`, `-Wextra`, `-std=c11`
  - Debug: `-O0`/`/Od`, `-g`/`/Zi`
  - Release: `-O3`/`/O2`
- **Include paths**: Includes `engine/src/main` for header access
- **Output**: `sandbox.exe` (Windows), `sandbox` (Linux/macOS)

## Code Architecture

### Platform Abstraction

TELEIOS uses a multi-layer platform detection and abstraction system:

**Platform Detection** ([engine/src/main/teleios/defines.h](engine/src/main/teleios/defines.h)):
- Detects OS: `TL_PLATFORM_WINDOWS`, `TL_PLATFORM_LINUX`, `TL_PLATFORM_APPLE`, etc.
- Detects POSIX compliance levels (XOPEN versions)
- Conditionally includes standard library headers based on C standard version

**Platform Implementation** ([engine/src/main/teleios/platform.h](engine/src/main/teleios/platform.h)):
- `tl_platform_initialize()` / `tl_platform_terminate()` - Platform setup/cleanup
- Implementations:
  - [platform_windows.c](engine/src/main/teleios/platform_windows.c)
  - [platform_linux.c](engine/src/main/teleios/platform_linux.c)
  - [platform_agnostic.c](engine/src/main/teleios/platform_agnostic.c) - Cross-platform code

### Type System

Custom type aliases defined in [defines.h](engine/src/main/teleios/defines.h:133-173):
- Unsigned: `u8`, `u16`, `u32`, `u64`
- Signed: `i8`, `i16`, `i32`, `i64`
- Float: `f32`, `f64`
- Boolean: `b8`

Static assertions verify type sizes at compile time.

### API Visibility

Uses compiler-specific attributes for DLL export/import:
- `TL_API` - Marks functions for export when `TELEIOS_EXPORT` is defined
- `TL_INLINE` / `TL_NOINLINE` - Force inlining control
- `TL_DEPRECATED(message)` - Mark deprecated functions

### Core Modules

**Application** ([teleios/application.h](engine/src/main/teleios/application.h)):
- `tl_application_initialize()` - Setup application
- `tl_application_run()` - Main game loop
- `tl_application_terminate()` - Cleanup

**Logger** ([teleios/logger.h](engine/src/main/teleios/logger.h)):
- Levels: VERBOSE, TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- Macros: `TLVERBOSE()`, `TLTRACE()`, `TLDEBUG()`, `TLINFO()`, `TLWARN()`, `TLERROR()`, `TLFATAL()`
- Debug-only logs (VERBOSE/TRACE/DEBUG) are compiled out in release builds

**Main Header** ([teleios/teleios.h](engine/src/main/teleios/teleios.h)):
- Single include for all engine functionality
- Includes: defines, chrono, logger, platform, filesystem

### Module Organization

All engine code lives under `engine/src/main/`:
- `teleios/*.{h,c}` - Core engine modules
- `glfw/*.h` - GLFW windowing library headers (vendored)
- `main.c` - Engine entry point

## Important Conventions

### Naming Convention
- **Build-time preprocessors**: `TELEIOS_*` (e.g., `TELEIOS_BUILD_DEBUG`, `TELEIOS_EXPORT`)
- **Runtime macros/constants**: `TL_<NAME>` (e.g., `TL_PLATFORM_LINUX`, `TL_API`)
- **Macro functions**: `TL<Name>` (e.g., `TLINFO(...)`, `TLERROR(...)`)
- **API functions**: `tl_module_function()` (e.g., `tl_platform_initialize()`, `tl_logger_write()`)
- **Type aliases**: lowercase (e.g., `u8`, `b8`, `f32`)

### Preprocessor Defines
- `TELEIOS_BUILD_DEBUG` - Debug mode (enables VERBOSE/TRACE/DEBUG logging macros)
- `TELEIOS_BUILD_RELEASE` - Release mode (compiles out debug-only logs for performance)
- `TELEIOS_EXPORT` - Marks functions for DLL export (engine only)

### Platform-Specific Code
When adding platform-specific implementations:
1. Add platform detection in `defines.h` if needed
2. Create or update `platform_<os>.c` with implementation
3. Ensure fallback in `platform_agnostic.c` if applicable
4. Use `#ifdef TL_PLATFORM_*` guards

## Compiler Configuration

The project supports multiple compilers:
- **C Standard**: C11 (`-std=c11`)
- **Warnings**: `-Wall`, `-Wextra`
- **Supported Compilers**: Clang, GCC, MSVC
- **Windows-specific**: CodeView debug format (`-gcodeview`) for Clang/GCC

CMake automatically detects and configures the compiler.

## Build Artifacts

Build outputs are organized in the `build/` directory:
- `build/bin/` - Executables
  - `engine.exe` / `engine` - Engine application
  - `sandbox.exe` / `sandbox` - Sandbox application
  - `*.pdb` - Debug symbols (Windows only)
- `build/lib/` - Static libraries
  - `glfw3.lib` / `libglfw3.a` - GLFW library (1.5 MB)
  - `yaml.lib` / `libyaml.a` - libyaml library (529 KB)
  - `lua.lib` / `liblua.a` - Lua library (1.3 MB)
- `build/_deps/` - Downloaded dependencies (GLFW, libyaml, Lua source and build)
- `build/CMakeFiles/` - CMake internal files and dependency tracking

CMake automatically tracks dependencies and performs incremental builds.

## Development Workflow

### Adding New Modules

When creating new engine modules:
1. Create header file in `engine/src/main/teleios/<module>.h`
2. Create implementation in `engine/src/main/teleios/<module>.c`
3. Add include to `engine/src/main/teleios/teleios.h` if it's a core module
4. Follow naming conventions: `tl_<module>_<function>()` for functions
5. Use `TL_API` macro for functions that need to be exported

### Modifying Build Configuration

To modify the build configuration:

**Adding source files:**
1. Add `.c` files to the appropriate `CMakeLists.txt`
2. Update the `ENGINE_SOURCES` or `SANDBOX_SOURCES` list
3. CMake will automatically detect and compile them

**Adding compiler flags:**
1. Edit `engine/CMakeLists.txt` or `sandbox/CMakeLists.txt`
2. Modify `target_compile_options()` section
3. Use generator expressions for configuration-specific flags: `$<$<CONFIG:Debug>:-g>`

**Adding linker flags:**
1. Edit the appropriate `CMakeLists.txt`
2. Modify `target_link_libraries()` or add `target_link_options()`

**Adding preprocessor defines:**
1. Edit the appropriate `CMakeLists.txt`
2. Modify `target_compile_definitions()` section
3. Use generator expressions for configuration-specific defines

**Adding dependencies:**
1. Use CMake's `FetchContent` module:
   - For libraries with proper CMake support: `FetchContent_MakeAvailable()`
   - For libraries needing manual build: `FetchContent_Populate()` + manual library creation
   - See libyaml and Lua examples in `engine/CMakeLists.txt` (lines 25-102)
2. Or use `find_package()` for system-installed libraries
3. Link with `target_link_libraries()`
4. Update `.vscode/c_cpp_properties.json` with include paths for IntelliSense

### Using External Libraries

All external libraries are statically linked with the engine. To use them in code:

**GLFW (windowing and input):**
```c
#include <GLFW/glfw3.h>
```

**libyaml (YAML parsing):**
```c
#include <yaml.h>
```

**Lua (scripting):**
```c
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
```

Include paths are automatically configured by CMake. No additional setup needed.

### Working with Platform-Specific Code

- Keep platform-agnostic code in shared files
- Use `#ifdef TL_PLATFORM_*` guards for platform-specific sections
- Implement platform-specific functions in separate files (`platform_windows.c`, `platform_linux.c`)
- Test builds on all target platforms when modifying platform code

## Running Executables

**Engine:**
```bash
# Windows
.\build\bin\Debug\engine.exe

# Linux/macOS
./build/bin/engine
```

**Sandbox:**
```bash
# Windows
.\build\bin\Debug\sandbox.exe

# Linux/macOS
./build/bin/sandbox
```
