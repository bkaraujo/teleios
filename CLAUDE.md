# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TELEIOS is a cross-platform game engine written in C11.
- **engine**: Core engine library providing game loop, rendering thread, and subsystems
- **sandbox**: Application/game that implements game-specific callbacks

## Build System

**CMake 3.20+** with automatic dependency management via FetchContent:
- **GLFW 3.4**: Windowing and input
- **libyaml 0.2.5**: YAML parser
- **Lua 5.4.7**: Scripting language
- **cglm 0.9.4**: Mathematics (vectors, matrices, quaternions)
- **GLAD**: OpenGL loader (vendored in `engine/src/main/glad/`)
- **STB Image**: Image loading (vendored in `engine/src/main/stb/`)

### Build Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --config Debug

# Run (executables auto-copied to dist/)
./dist/engine.exe  # Windows
./dist/engine      # Linux/macOS
```

### Build Configuration

**Engine**: Standalone executable
- Defines: `TELEIOS_BUILD_DEBUG` / `TELEIOS_BUILD_RELEASE`
- Links: GLFW, libyaml, Lua, cglm, platform libs (user32, gdi32, opengl32, pthread, etc.)
- Flags: `/W4 /std:c11` (MSVC), `-Wall -Wextra -std=c11` (GCC/Clang)

**Sandbox**: Shared library (DLL/SO)
- Similar configuration, lighter dependencies
- Provides callbacks: `tl_application_initialize()`, `tl_application_run()`, `tl_application_terminate()`

Build outputs in `build/bin/`, auto-copied to `dist/`.

## Configuration System

### application.yml

The engine loads `sandbox/application.yml` for runtime configuration:

```yaml
teleios:
  logging:
    level: debug          # Logging verbosity
  graphics:
    vsync: false         # V-sync on/off
    wireframe: false     # Wireframe rendering
  simulation:
    step: 20s            # Fixed timestep in milliseconds (20ms = 50 Hz)
  window:
    size: SD             # Display resolution
    title: LearnOpenGL   # Window title

application:
  scene:
    - name: main
      clear_color: .7, .2, .7, 1  # RGBA clear color
      camera: { ... }              # Camera configuration
      actor:                       # Scene entities
        - name: "Ball"
        - name: "Paddle Left"
```

**Key Configuration:**
- `simulation.step`: Fixed timestep for physics (default: 20ms)
- `logging.level`: Controls which log messages appear
- `graphics.vsync`: Affects frame pacing

## Testing

**Current Status**: No test infrastructure exists in the codebase.

To add tests:
1. Create `engine/tests/` directory
2. Add test framework (Unity, Cmocka, or custom)
3. Update `engine/CMakeLists.txt` with `enable_testing()` and `add_test()`
4. Run with `ctest`

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

- `TL_API` - Export marker for public functions
- `TL_INLINE` / `TL_NOINLINE` - Inlining control
- `TL_THREADLOCAL` - Thread-local storage
- `TL_LIKELY(x)` / `TL_UNLIKELY(x)` - Branch prediction hints

## Core Modules

### Application
- Fixed timestep game loop with accumulator pattern
- Configurable via `application.yml` (default: 20ms = 50 Hz)
- Spiral of death protection (caps max frame time at 250ms)
- FPS/UPS tracking (logged every second)

### Logger
- Levels: VERBOSE, TRACE, DEBUG, INFO, WARN, ERROR, FATAL
- Macros: `TLVERBOSE()`, `TLTRACE()`, `TLDEBUG()`, `TLINFO()`, `TLWARN()`, `TLERROR()`, `TLFATAL()`
- Thread-safe with `TL_THREADLOCAL` timestamp caching
- Debug logs compiled out in release builds

### Profiler
- Stack-based call tracking (max 1000 frames, configurable via `TELEIOS_FRAME_MAXIMUM`)
- Macros: `TL_PROFILER_PUSH`, `TL_PROFILER_PUSH_WITH(args)`, `TL_PROFILER_POP`, `TL_PROFILER_POP_WITH(value)`
- Fatal error on stack overflow (no graceful degradation)

### Memory
- **LINEAR**: Fast bump allocator, no individual free, bulk deallocation
- **DYNAMIC**: Heap allocator with leak detection (debug mode)
- `tl_memory_allocator_create(size, type)` - Create allocator
- `tl_memory_alloc(allocator, tag, size)` - Tagged allocations for profiling
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
- **Dedicated rendering thread** (all OpenGL on worker thread)
- Thread-safe work queue (capacity: 512 jobs)
- Task pool (512 pre-allocated tasks, zero runtime allocations)
- API: `tl_graphics_submit_sync()`, `tl_graphics_submit_sync_args()`, `tl_graphics_submit_async()`, `tl_graphics_submit_async_args()`
- **Structure**: `graphics.c` includes `graphics/types.inl`, `graphics/queue.inl`, `graphics/thread.inl`
- **CRITICAL**: All OpenGL calls MUST go through graphics thread submission

### Event System
- Subscriber pattern: `tl_event_subscribe(type, handler)`
- `tl_event_submit(type, data)` - Dispatch to all handlers
- Handler return: `TL_EVENT_CONSUMED` stops propagation, `TL_EVENT_AVAILABLE` continues

### Container
- **Queue**: Ring buffer, fixed capacity, NOT thread-safe (external locking required)
- **Pool**: Pre-allocated objects, bitmap tracking, thread-safe
- **List**: Double linked list, O(1) insert/remove, thread-safe
- **Map**: Hash map (TLString -> TLList*), FNV-1a hash, auto-resize at 75% load, thread-safe
- **Iterator**: Fail-fast, snapshot-based, lock-free after creation
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
- Task pool: 512 pre-allocated tasks
- Each task has pre-allocated mutex + condition variable
- Sync jobs: stack-allocated args OK
- Async jobs: heap/static args required

## Quick Reference

### Creating a New Module

```bash
# 1. Create directory and files
mkdir engine/src/main/teleios/mymodule
touch engine/src/main/teleios/mymodule.h
touch engine/src/main/teleios/mymodule.c
touch engine/src/main/teleios/mymodule/types.inl
touch engine/src/main/teleios/mymodule/variant.inl

# 2. Update CMakeLists.txt
# Add src/main/teleios/mymodule.c to ENGINE_SOURCES

# 3. Include in teleios.h if core module
```

### Adding a New Allocator Type

```c
// 1. Add enum to memory.h
typedef enum {
    TL_ALLOCATOR_LINEAR,
    TL_ALLOCATOR_DYNAMIC,
    TL_ALLOCATOR_POOL      // New type
} TLAllocatorType;

// 2. Create memory/pool.inl
#ifndef __TELEIOS_MEMORY_POOL__
#define __TELEIOS_MEMORY_POOL__
#include "teleios/memory/types.inl"

static void* tl_memory_pool_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size) {
    // Implementation
}

static void tl_memory_pool_free(TLAllocator* allocator, void* pointer) {
    // Implementation
}
#endif

// 3. Update memory.c dispatcher
#include "teleios/memory/pool.inl"

void* tl_memory_alloc(...) {
    switch (allocator->type) {
        case TL_ALLOCATOR_POOL:
            memory = tl_memory_pool_alloc(allocator, tag, size);
            break;
        // ... other cases
    }
}
```

### Submitting Graphics Work

```c
// Setup (synchronous - blocks until complete)
static void* setup_opengl(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    return NULL;
}
tl_graphics_submit_sync(setup_opengl);

// Render loop (asynchronous - returns immediately)
static void* render_frame(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ... rendering commands
    glfwSwapBuffers(tl_window_handler());
    return NULL;
}

while (running) {
    glfwPollEvents();  // Main thread only
    tl_graphics_submit_async(render_frame);
}
```

### Using Containers

```c
// Queue (thread-unsafe - use external locking)
TLQueue* queue = tl_queue_create(allocator, 256);
tl_queue_push(queue, my_data);
void* data = tl_queue_pop(queue);

// Object Pool (thread-safe)
TLObjectPool* pool = tl_pool_create(allocator, sizeof(MyStruct), 100);
MyStruct* obj = tl_pool_acquire(pool);
// ... use object
tl_pool_release(pool, obj);

// List (thread-safe)
TLList* list = tl_list_create(allocator);
tl_list_push_back(list, my_data);
TLIterator* it = tl_list_iterator(list);
while (tl_iterator_has_next(it)) {
    void* data = tl_iterator_next(it);
}
tl_iterator_destroy(it);
```

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

**Logger**: Thread-local timestamp caching, debug logs compiled out in release

**Profiler**: 1000 frame stack depth, ~1MB static allocation, not for production

**Containers**:
- Queue: O(1) enqueue/dequeue
- Pool: O(1) acquire/release (typically)
- List: O(1) insert/remove
- Map: O(1) avg get/put, auto-resize at 75% load
