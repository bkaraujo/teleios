# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TELEIOS is a cross-platform game engine written in C11. The project consists of two main components:
- **engine**: The core engine library with platform abstraction layers
- **sandbox**: Application/game that uses the engine

## Build System

The project uses a modular PowerShell-based build system with incremental compilation support.

### Building the Project

```powershell
# Build everything (engine + sandbox)
.\build_all.ps1

# Clean build
.\build_all.ps1 -Clean

# Build only engine
.\build_all.ps1 -EngineOnly
# or
.\engine\build.ps1

# Build only sandbox
.\build_all.ps1 -SandboxOnly
# or
.\sandbox\build.ps1
```

### Build System Architecture

The build system is layered:
1. **build/compile.ps1** - Core compilation script (accepts flags as parameters)
2. **engine/build.ps1** - Configures engine-specific flags and calls compile.ps1
3. **sandbox/build.ps1** - Configures sandbox-specific flags and calls compile.ps1
4. **build_all.ps1** - Orchestrates building both projects

Key build parameters:
- `-CompileFlags`: Array of compilation flags (e.g., `-O0`, `-g`)
- `-LinkFlags`: Array of linker flags (e.g., `-luser32`)
- `-Defines`: Array of preprocessor defines (e.g., `TELEIOS_DEBUG`)

### Parallel Compilation

The build system automatically detects the number of physical CPU cores and compiles files in parallel using PowerShell RunspacePool for maximum performance. Dependencies between C files (via `#include "file.c"`) are tracked to ensure correct build order.

### Engine Build Configuration

The engine is built with:
- **Defines**: `TELEIOS_EXPORT` (for DLL exports), `TELEIOS_BUILD_DEBUG`
- **Link flags**: `-luser32`, `-lgdi32`, `-lkernel32`, `-lopengl32`
- **Compile flags**: `-O0` (no optimization), `-g` (debug symbols), `-gcodeview` (Windows debug format)
- Debug symbols enabled for debugging with Visual Studio or other debuggers

### Sandbox Build Configuration

The sandbox is built with:
- **Defines**: `TELEIOS_BUILD_DEBUG`
- **Link flags**: `-luser32`, `-lgdi32`, `-lkernel32`
- **Compile flags**: `-O0`, `-g`, `-gcodeview`, `-I<engine>/src/main` (auto-includes engine headers)
- The sandbox automatically includes engine headers for seamless integration

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

The project uses Clang with:
- C11 standard (`-std=c11`)
- Warnings enabled (`-Wall`, `-Wextra`)
- Windows-specific: CodeView debug format (`-gcodeview`)

## Build Artifacts

Each project creates:
- `<project>/build/obj/` - Object files (.o)
- `<project>/build/deps/` - Dependency files (.d) for incremental builds
- `<project>/<project>.exe` - Final executable

The build system tracks dependencies automatically and only recompiles files when source or headers change.

## Development Workflow

### Adding New Modules

When creating new engine modules:
1. Create header file in `engine/src/main/teleios/<module>.h`
2. Create implementation in `engine/src/main/teleios/<module>.c`
3. Add include to `engine/src/main/teleios/teleios.h` if it's a core module
4. Follow naming conventions: `tl_<module>_<function>()` for functions
5. Use `TL_API` macro for functions that need to be exported

### Modifying Build Configuration

To add custom compiler or linker flags:
1. Edit `engine/build.ps1` or `sandbox/build.ps1` depending on the project
2. Modify the appropriate array: `$CompileFlags`, `$LinkFlags`, or `$Defines`
3. The main `build/compile.ps1` script automatically uses these flags

### Working with Platform-Specific Code

- Keep platform-agnostic code in shared files
- Use `#ifdef TL_PLATFORM_*` guards for platform-specific sections
- Implement platform-specific functions in separate files (`platform_windows.c`, `platform_linux.c`)
- Test builds on all target platforms when modifying platform code
