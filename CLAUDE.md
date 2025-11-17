# CLAUDE.md

This file provides guidance to Claude Code when working with this repository.

## Project Overview

TELEIOS is a cross-platform game engine written in C11.
- **engine**: Core engine library with platform abstraction layers
- **sandbox**: Application/game using the engine

## Build System

**CMake 3.20+** with automatic dependency management via FetchContent:
- **GLFW 3.4**: Windowing and input
- **libyaml 0.2.5**: YAML parser
- **Lua 5.4.7**: Scripting language
- **cglm 0.9.4**: Mathematics (vectors, matrices, quaternions)
- **GLAD**: OpenGL loader (vendored in `engine/src/main/glad/`)
- **STB Image**: Image loading (vendored in `engine/src/main/stb/`)

### Build Configuration

**Engine**: Standalone executable
- Defines: `TELEIOS_BUILD_DEBUG` / `TELEIOS_BUILD_RELEASE`
- Links: GLFW, libyaml, Lua, cglm, platform libs (user32, gdi32, opengl32, pthread, etc.)
- Flags: `/W4 /std:c11` (MSVC), `-Wall -Wextra -std=c11` (GCC/Clang)

**Sandbox**: Standalone executable
- Similar configuration, lighter dependencies

Build outputs in `build/bin/`, auto-copied to `dist/`.

## Code Architecture

### Modular Structure

All engine modules follow a **modular .inl pattern** for separation of concerns:

```
engine/src/main/teleios/
├── <module>/
│   ├── types.inl          # Shared type definitions
│   ├── <variant>.inl      # Platform/type-specific implementations
│   └── ...
├── <module>.h             # Public API
├── <module>.c             # Dispatcher (includes .inl files)
└── main.c
```

**Current modules using this pattern:**
- `memory/` - types.inl, linear.inl, dynamic.inl
- `platform/` - types.inl, windows.inl, linux.inl, glfw.inl
- `thread/` - windows.inl, unix.inl
- `graphics/` - types.inl, queue.inl, thread.inl
- `container/` - types.inl, queue.inl, pool.inl, list.inl, map.inl, iterator.inl

### Type System

Defined in `teleios/defines.h`:
- Unsigned: `u8`, `u16`, `u32`, `u64`
- Signed: `i8`, `i16`, `i32`, `i64`
- Float: `f32`, `f64`, Boolean: `b8`

### API Macros

- `TL_API` - Export marker
- `TL_INLINE` / `TL_NOINLINE` - Inlining control
- `TL_THREADLOCAL` - Thread-local storage
- `TL_LIKELY(x)` / `TL_UNLIKELY(x)` - Branch hints

## Core Modules

### Application
- Fixed timestep game loop with accumulator pattern
- Spiral of death protection (250ms cap)
- FPS/UPS tracking

### Logger
- Levels: VERBOSE, TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- Macros: `TLVERBOSE()`, `TLTRACE()`, `TLDEBUG()`, `TLINFO()`, `TLWARN()`, `TLERROR()`, `TLFATAL()`
- Thread-safe with `TL_THREADLOCAL` timestamp caching
- Debug logs compiled out in release builds

### Profiler
- Stack-based call tracking (max 1000 frames, configurable via `TELEIOS_FRAME_MAXIMUM`)
- Macros: `TL_PROFILER_PUSH`, `TL_PROFILER_PUSH_WITH(args)`, `TL_PROFILER_POP`, `TL_PROFILER_POP_WITH(value)`

### Memory
- `tl_memory_allocator_create(size, type)` - LINEAR or DYNAMIC allocators
- `tl_memory_alloc(allocator, tag, size)` - Tagged allocations for tracking
- Custom allocators per subsystem for cache efficiency
- **Structure**: `memory.c` dispatches to `memory/linear.inl` or `memory/dynamic.inl`

### Platform
- `tl_platform_initialize()` / `tl_platform_terminate()`
- Timing: QueryPerformanceCounter (Windows), clock_gettime (Linux)
- Window management via GLFW
- **Structure**: `platform.c` includes `platform/windows.inl`, `platform/linux.inl`, `platform/glfw.inl`

### Thread
- Thread: create, join, detach, sleep
- Mutex: create, destroy, lock, unlock, trylock
- Condition variables: create, destroy, wait, signal, broadcast
- **Structure**: `thread.c` includes `thread/windows.inl` or `thread/unix.inl`

### Graphics
- Dedicated rendering thread (all OpenGL on worker thread)
- Thread-safe work queue (capacity: 512 jobs)
- Task pool (512 pre-allocated tasks, zero runtime allocations)
- API: `tl_graphics_submit_sync()`, `tl_graphics_submit_sync_args()`, `tl_graphics_submit_async()`, `tl_graphics_submit_async_args()`
- **Structure**: `graphics.c` includes `graphics/types.inl`, `graphics/queue.inl`, `graphics/thread.inl`
- **CRITICAL**: All OpenGL calls MUST go through graphics thread submission

### Event System
- Subscriber pattern: `tl_event_subscribe(type, handler)`
- `tl_event_submit(type, data)` - Dispatch to all handlers
- Handler return: `TL_EVENT_CONSUMED` stops propagation

### Container
- **Queue**: Ring buffer, fixed capacity, thread-unsafe
- **Pool**: Pre-allocated objects, bitmap tracking, thread-safe
- **List**: Double linked list, O(1) insert/remove, thread-safe
- **Map**: Hash map (TLString -> TLList*), FNV-1a hash, thread-safe
- **Iterator**: Snapshot-based, lock-free traversal
- **Structure**: `container.c` includes all `container/*.inl` files

### Strings
- Immutable `TLString` with cached length
- Operations: create, concat, substring, trim, replace, split
- `TLStringBuilder` for efficient incremental building

## Naming Conventions

- **Build-time**: `TELEIOS_*` (e.g., `TELEIOS_BUILD_DEBUG`)
- **Runtime macros**: `TL_<NAME>` (e.g., `TL_PLATFORM_LINUX`)
- **Macro functions**: `TL<Name>` (e.g., `TLINFO(...)`)
- **API functions**: `tl_module_function()` (e.g., `tl_platform_initialize()`)
- **Type aliases**: lowercase (e.g., `u8`, `b8`, `f32`)

## Modular Architecture Pattern

**All complex subsystems follow this pattern:**

```
<module>.h              # Public API
<module>.c              # Dispatcher (type/platform routing)
<module>/types.inl      # Shared type definitions
<module>/<variant>.inl  # Specialized implementations
```

**Dispatcher uses switch/case for type routing:**

```c
void* tl_memory_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size) {
    void* memory = NULL;
    switch (allocator->type) {
        case TL_ALLOCATOR_LINEAR:
            memory = tl_memory_linear_alloc(allocator, tag, size);
            break;
        case TL_ALLOCATOR_DYNAMIC:
            memory = tl_memory_dynamic_alloc(allocator, tag, size);
            break;
        default:
            TLFATAL("Unsupported Allocator type");
    }
    return memory;
}
```

**When to use:**
1. Multiple implementations for same interface
2. Platform-specific code
3. Type-based dispatch at runtime
4. Code size > 100 lines
5. Future extensibility needed

**Implementation rules:**
- `.inl` functions are `static` by default (unless called from dispatcher)
- Helper functions in dispatcher use non-static linkage
- Use `extern` declarations for cross-file helpers
- Include order: types.inl → specialized.inl → dispatcher includes all
- Always include `default:` case with `TLFATAL`

### `.inl` Linkage Rules

**Functions callable from other compilation units:**
- MUST NOT be `static` or `TL_INLINE`
- Example: `tl_window_handler()` called from `application.c`, `graphics.c`

**Internal helper functions:**
- SHOULD be `static`
- Example: GLFW callbacks, internal initialization

## Graphics Threading

**Threading Model:**
```
Main Thread                Graphics Thread
-----------                ---------------
• Game logic               • OpenGL context ownership
• glfwPollEvents()        • GLAD initialization
• Submit commands ──────► • Process queue
                          • Exit on TL_EVENT_WINDOW_CLOSED
```

**Rules:**
1. Main thread NEVER acquires OpenGL context
2. Graphics thread owns context from startup
3. GLAD must initialize AFTER `glfwMakeContextCurrent()`
4. `glfwPollEvents()` on main thread only
5. All OpenGL via `tl_graphics_submit_*()`

**Memory:**
- Graphics thread has dedicated DYNAMIC allocator
- Task pool: 512 pre-allocated tasks (28KB)
- Each task has pre-allocated mutex + condition variable
- Sync jobs: stack-allocated args OK
- Async jobs: heap/static args required

## Development Workflow

### Adding New Modules

1. Create `teleios/<module>.h` (public API)
2. Create `teleios/<module>/` directory
3. Create `teleios/<module>/types.inl` (shared types)
4. Create `teleios/<module>/<variant>.inl` (implementations)
5. Create `teleios/<module>.c` (dispatcher)
6. Add to `engine/CMakeLists.txt` ENGINE_SOURCES
7. Include in `teleios/teleios.h` if core module

### Build Modifications

**Add sources**: Update `ENGINE_SOURCES` in `CMakeLists.txt`
**Add flags**: Modify `target_compile_options()`
**Add defines**: Modify `target_compile_definitions()`
**Add deps**: Use `FetchContent` or `find_package()`

### External Libraries Usage

```c
#include <GLFW/glfw3.h>       // Windowing
#include <yaml.h>             // YAML parsing
#include <lua.h>              // Scripting
#include <cglm/cglm.h>        // Mathematics
#include <glad/glad.h>        // OpenGL loader (before GLFW)
#include <stb/stb_image.h>    // Image loading
```

## Performance Notes

**Logger**: ~820-1,930 ns per log
- Timestamp syscall is main bottleneck (50-60%)
- Thread-local caching optimizes repeated calls
- Debug logs compiled out in release

**Profiler**: 1000 frame stack depth
- ~1MB static allocation
- `TL_LIKELY`/`TL_UNLIKELY` hints for optimization
- Not for production (use Tracy/Optick instead)

**Containers**:
- Queue: O(1) enqueue/dequeue
- Pool: O(1) acquire/release (typically)
- List: O(1) insert/remove
- Map: O(1) avg get/put, auto-resize at 75% load

## Running

```bash
# Windows
.\dist\engine.exe

# Linux/macOS
./dist/engine
```

Build artifacts auto-copied to `dist/` directory.
