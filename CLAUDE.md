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

**cglm 0.9.4** - OpenGL Mathematics library
- Automatically downloaded from tar.gz
- Built as static library (header-only style with compiled objects)
- Provides vector/matrix operations (vec2/3/4, mat2/3/4, quaternions)
- Linked statically with both engine and sandbox

**GLAD** - OpenGL loader
- Vendored in `engine/src/main/glad/` (glad.c, glad.h, khrplatform.h)
- Loads OpenGL function pointers at runtime
- Compiled directly with engine

**STB Image** - Image loading library
- Vendored as single-header library in `engine/src/main/stb/stb_image.h`
- Supports PNG, JPG, BMP, TGA, PSD, GIF, HDR, PIC formats
- Header-only, no compilation needed

No manual dependency installation required - just run `.\build.ps1`

### Engine Build Configuration

The engine is built as a standalone executable with:
- **Defines**:
  - Debug: `TELEIOS_BUILD_DEBUG`
  - Release: `TELEIOS_BUILD_RELEASE`
- **Link libraries**:
  - Cross-platform: GLFW, libyaml (`yaml`), Lua (`lua_static`), cglm
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
  - Cross-platform: GLFW, cglm
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
- Platform-specific code uses `.inl` files included directly into `platform.c`:
  - [platform_windows.inl](engine/src/main/teleios/platform_windows.inl) - Windows (QueryPerformanceCounter timing)
  - [platform_linux.inl](engine/src/main/teleios/platform_linux.inl) - Linux (clock_gettime timing)
  - [platform_glfw.inl](engine/src/main/teleios/platform_glfw.inl) - GLFW window management (cross-platform)

**Threading Implementation** ([engine/src/main/teleios/thread.h](engine/src/main/teleios/thread.h)):
- Platform-specific code uses `.inl` files included directly into `thread.c`:
  - [thread_windows.inl](engine/src/main/teleios/thread_windows.inl) - Windows threads and synchronization primitives
  - [thread_unix.inl](engine/src/main/teleios/thread_unix.inl) - POSIX pthread implementation

**Platform Implementation Pattern:**
- `.inl` files are **included** (not compiled separately) to allow static functions without header pollution
- Windows timing uses QueryPerformanceCounter with pre-calculated multiplier/shift to avoid division in hot path
- Linux timing uses `CLOCK_REALTIME_COARSE` for milliseconds and `CLOCK_REALTIME` for microseconds
- Thread implementations abstract Windows native threads/mutexes and POSIX pthreads under unified API

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
- `TL_THREADLOCAL` - Thread-local storage (critical for multithreading safety)
- `TL_LIKELY(x)` / `TL_UNLIKELY(x)` - Branch prediction hints for optimization

### Core Modules

**Application** ([teleios/application.h](engine/src/main/teleios/application.h)):
- `tl_application_initialize()` - Setup application
- `tl_application_run()` - Main game loop with fixed timestep
- `tl_application_terminate()` - Cleanup

**Game Loop Architecture:**
- **Fixed timestep**: 60 Hz (16.667ms per update)
- **Accumulator-based**: Decouples simulation from rendering for consistent physics
- **Spiral of death protection**: Caps max frame time at 250ms
- **Interpolation support**: Alpha value computed for smooth rendering between updates
- **Performance tracking**: FPS (Frames Per Second) and UPS (Updates Per Second) logged every second

**Logger** ([teleios/logger.h](engine/src/main/teleios/logger.h)):
- Levels: VERBOSE, TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- Macros: `TLVERBOSE()`, `TLTRACE()`, `TLDEBUG()`, `TLINFO()`, `TLWARN()`, `TLERROR()`, `TLFATAL()`
- Debug-only logs (VERBOSE/TRACE/DEBUG) are compiled out in release builds
- **Highly optimized**: ~820-1,930 ns per log with thread-local timestamp caching
- **Color-coded output**: ANSI color codes for each log level
- **Thread-safe**: Uses `TL_THREADLOCAL` for safe concurrent logging

**Profiler** ([teleios/profiler.h](engine/src/main/teleios/profiler.h)):
- Stack-based call tracking (max 1000 nested frames, configurable via `TELEIOS_FRAME_MAXIMUM`)
- Macros: `TL_PROFILER_PUSH`, `TL_PROFILER_PUSH_WITH(args)`, `TL_PROFILER_POP`, `TL_PROFILER_POP_WITH(value)`
- Stores filename, function name, line number, and optional formatted arguments
- Integrates with logger for "[in]" / "[out]" trace messages
- Uses `TL_LIKELY`/`TL_UNLIKELY` branch hints for optimization
- Arguments buffer: 1024 bytes per frame (configurable via `TL_PROFILER_FRAME_ARGUMENTS_SIZE`)

**Window** ([teleios/window.h](engine/src/main/teleios/window.h)):
- `tl_window_handler()` - Get GLFW window pointer
- `tl_window_size()` - Query window dimensions
- `tl_window_position()` - Query window position
- Window created during `tl_platform_initialize()` with OpenGL 4.6 Core Profile
- Default: 1024x768, centered on primary monitor, initially invisible

**Chrono** ([teleios/chrono.h](engine/src/main/teleios/chrono.h)):
- `tl_time_clock()` - Get current date and time (fills TLDateTime struct)
- `tl_time_epoch_millis()` - Get milliseconds since Unix epoch
- `tl_time_epoch_micros()` - Get microseconds since Unix epoch
- `TLDateTime` struct: Contains year, month, day, hour, minute, second, and milliseconds
- Used internally by the logger for timestamps

**Filesystem** ([teleios/filesystem.h](engine/src/main/teleios/filesystem.h)):
- `tl_filesystem_path_separator()` - Returns platform-specific path separator ('\\' on Windows, '/' on Unix)
- Provides cross-platform file path utilities

**Memory** ([teleios/memory.h](engine/src/main/teleios/memory.h)):
- `tl_memory_initialize()` / `tl_memory_terminate()` - Initialize/cleanup memory system
- `tl_memory_allocator_create()` / `tl_memory_allocator_destroy()` - Create/destroy custom allocators
- `tl_memory_alloc()` - Allocate memory with tag for tracking
- `tl_memory_set()` / `tl_memory_copy()` - Memory manipulation utilities
- **Tagged allocations**: Memory tags (e.g., `TL_MEMORY_THREAD`, `TL_MEMORY_SCENE`) for debugging and profiling
- **Custom allocators**: Per-subsystem memory pools for cache efficiency

**Thread** ([teleios/thread.h](engine/src/main/teleios/thread.h)):
- Thread management:
  - `tl_thread_create()` - Create and start a new thread
  - `tl_thread_join()` - Wait for thread completion and retrieve result
  - `tl_thread_detach()` - Detach thread for automatic cleanup
  - `tl_thread_id()` - Get current thread ID
  - `tl_thread_sleep()` - Sleep for specified milliseconds
- Mutex (mutual exclusion):
  - `tl_mutex_create()` / `tl_mutex_destroy()` - Create/destroy mutex
  - `tl_mutex_lock()` / `tl_mutex_unlock()` - Lock/unlock mutex
  - `tl_mutex_trylock()` - Non-blocking lock attempt
- Condition variables:
  - `tl_condition_create()` / `tl_condition_destroy()` - Create/destroy condition variable
  - `tl_condition_wait()` / `tl_condition_wait_timeout()` - Wait on condition
  - `tl_condition_signal()` / `tl_condition_broadcast()` - Signal waiting threads
- **Cross-platform**: Abstracts Windows threads/mutexes and POSIX pthread API
- **Platform implementation**: Uses `.inl` files (thread_windows.inl, thread_unix.inl) included in thread.c

**Graphics** ([teleios/graphics.h](engine/src/main/teleios/graphics.h)):
- `tl_graphics_initialize()` - Initialize graphics system (GLAD OpenGL loader)
- `tl_graphics_terminate()` - Cleanup graphics resources
- **OpenGL context management**: Makes window's OpenGL context current
- **GLAD integration**: Loads OpenGL function pointers via `glfwGetProcAddress`
- **Version logging**: Reports OpenGL version and CGLM version at initialization
- **IMPORTANT**: GLAD must be included before GLFW in any file using OpenGL functions

**Event System** ([teleios/event.h](engine/src/main/teleios/event.h)):
- `tl_event_subscribe()` - Register handler for event type (max 255 handlers per event)
- `tl_event_submit()` - Dispatch event to all registered handlers
- **Event types**: Window events (close, resize, move, focus, minimize, maximize), input events (keyboard, mouse)
- **Event data**: Union type `TLEvent` can hold various data types (i8/u8/i16/u16/i32/u32/i64/u64/f32/f64)
- **Handler return**: `TL_EVENT_CONSUMED` stops propagation, `TL_EVENT_AVAILABLE` continues to next handler
- **GLFW integration**: Window callbacks in `platform_glfw.inl` automatically submit events
- **Subscriber pattern**: Decouples input/window handling from game logic

**Main Header** ([teleios/teleios.h](engine/src/main/teleios/teleios.h)):
- Single include for all engine functionality
- Includes: defines, profiler, platform, memory, window, thread, chrono, logger, filesystem, graphics, event

### Module Organization

All engine code lives under `engine/src/main/`:
- `teleios/*.{h,c}` - Core engine modules
- `glad/*.{h,c}` - GLAD OpenGL loader (vendored)
- `stb/*.h` - STB single-header libraries (stb_image.h)
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

### `.inl` File Pattern and Linkage Rules

**What are `.inl` files?**
- Platform-specific implementations included directly into `.c` files (not compiled separately)
- Allows `static` functions without polluting the global namespace
- Examples: `platform_windows.inl`, `platform_linux.inl`, `platform_glfw.inl`, `thread_windows.inl`, `thread_unix.inl`

**Important linkage rules:**
- **Functions in `.inl` files callable from other compilation units MUST NOT be `static` or `TL_INLINE`**
- **Functions in `.inl` files used only locally SHOULD be `static`**
- Incorrect linkage causes "unresolved external symbol" linker errors

**Example from `platform_glfw.inl`:**
```c
// ✅ CORRECT - External linkage for cross-file calls
void* tl_window_handler() {
    return m_window;
}

// ✅ CORRECT - Static for internal use only
static b8 tl_window_create(void) {
    // ...
}

// ❌ WRONG - Inline prevents cross-file linking
TL_INLINE void* tl_window_handler() {
    return m_window;  // Linker error!
}
```

**When to use each linkage:**
- **No modifier** (external): Functions called from other `.c` files (e.g., `tl_window_handler()` called from `application.c`, `graphics.c`)
- **`static`**: Helper functions used only within the `.inl` file (e.g., `tl_window_create()`, GLFW callbacks)
- **`TL_INLINE`**: Never use in `.inl` files for functions with external callers

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
  - `cglm.lib` / `libcglm.a` - cglm mathematics library
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

**cglm (mathematics):**
```c
#include <cglm/cglm.h>
```

**GLAD (OpenGL loader):**
```c
#include <glad/glad.h>
// Must be included before GLFW/glfw3.h
// Initialize with: gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)
```

**STB Image (image loading):**
```c
#define STB_IMAGE_IMPLEMENTATION  // Define once in exactly one .c file
#include <stb/stb_image.h>
// Usage: stbi_load(), stbi_image_free()
```

Include paths are automatically configured by CMake. No additional setup needed.

### Working with Platform-Specific Code

- Keep platform-agnostic code in shared files
- Use `#ifdef TL_PLATFORM_*` guards for platform-specific sections
- Implement platform-specific functions in separate files (`platform_windows.c`, `platform_linux.c`)
- Test builds on all target platforms when modifying platform code

## Performance Considerations

### Logger Performance

The logger is highly optimized with measured timings per log call:
- Early return check: ~1 ns
- Message formatting (`vsnprintf`): ~100-500 ns (variable)
- Basename extraction (`strrchr` x2): ~15-30 ns (SIMD-optimized)
- **Timestamp syscall**: ~500-1,000 ns (**50-60% of total time - main bottleneck**)
- Buffer formatting (`snprintf`): ~150-300 ns
- Direct write (`fwrite`): ~50-100 ns
- **Total**: ~820-1,930 ns per log

**Key optimizations implemented**:
- `fflush()` removed - OS manages buffer flushing for better performance
- Thread-local storage (`TL_THREADLOCAL`) for timestamp cache
- `strrchr()` uses hardware SIMD instructions on modern CPUs
- `fwrite()` instead of `fprintf()` for faster output
- Debug-only logs compiled out in Release builds (no runtime overhead)

**Known limitation**: Timestamp syscall dominates logging time. For extremely high-frequency logging (>1M logs/second), consider caching timestamps at reduced precision.

### Profiler Performance and Limitations

**Performance optimizations:**
- Static allocation avoids malloc/free overhead (~1.03MB total memory for 1000 frames)
- Branch prediction hints (`TL_LIKELY`/`TL_UNLIKELY`) optimize common paths
- Direct pointer assignment for filename/function (no string copy)
- Arguments buffer: 1024 bytes per frame (configurable via `TL_PROFILER_FRAME_ARGUMENTS_SIZE`)

**Configuration:**
- **Stack depth: 1000 frames** (configurable via `TELEIOS_FRAME_MAXIMUM`, default changed from 10 to 1000)
- **Memory usage**: ~1KB per frame (1024 bytes arguments + metadata)
- **Fatal error on overflow**: Application terminates with `TLFATAL` if limit exceeded
- **No graceful degradation**: Cannot handle call stacks deeper than configured limit

**Suitable for:**
- Deep call stack analysis (up to 1000 levels)
- Recursive algorithms with moderate depth
- Complex call chains in large codebases
- Development-time performance profiling
- Entry/exit logging for debugging

**Not suitable for:**
- Extremely deep recursion (>1000 levels, e.g., pathological cases)
- Production builds (overhead of logging and memory usage)
- Real-time profiling with minimal overhead (consider external tools like Tracy, Optick)

### Game Loop Performance

- Fixed 60 Hz update rate ensures consistent simulation
- Accumulator pattern allows rendering faster than 60 FPS without physics jitter
- Spiral of death protection prevents accumulator overflow in extreme lag scenarios
- Windows timing: QueryPerformanceCounter optimized with pre-calculated multiplier
- Linux timing: Direct `clock_gettime()` syscalls

### Thread Safety

- Logger uses `TL_THREADLOCAL` for thread-safe timestamp caching
- Profiler stack is thread-local by default
- Platform timing functions are thread-safe (read-only after initialization)

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
