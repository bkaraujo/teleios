# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

TELEIOS is a cross-platform game engine written in C11.

## Build Commands

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
./dist/engine.exe  # Windows | ./dist/engine (Linux/macOS)
```

**Testing:**
```bash
ctest --test-dir build --output-on-failure  # Run all tests
```

## File Structure

```
engine/src/main/teleios/
├── <module>/
│   ├── types.inl          # Shared type definitions
│   ├── <variant>.inl      # Platform/type-specific implementations
│   └── <variant>_safe.inl # Thread-safe variant (containers)
├── <module>.h             # Public API
├── <module>.c             # Dispatcher (includes .inl files)
├── defines.h              # Type aliases and macros
├── teleios.h              # Aggregates all public headers
└── main.c

sandbox/
├── application.yml        # Runtime configuration
└── scripts/               # Lua scripts per scene
    └── <scene>/
        ├── load.lua       # Executed on scene activation
        ├── unload.lua     # Executed on scene deactivation
        ├── frame_begin.lua # Executed AFTER engine frame_begin
        └── frame_end.lua   # Executed BEFORE swap buffers
```

**Modules:**
- `memory/` - linear.inl, dynamic.inl
- `platform/` - windows.inl, linux.inl, glfw.inl
- `thread/` - thread.inl, mutex.inl, condition.inl, types.inl
- `graphics/` - Dedicated rendering thread
- `container/` - queue.inl, pool.inl, list.inl, map.inl, array.inl, iterator.inl

## Naming Conventions

| Category | Pattern | Example |
|----------|---------|---------|
| Build-time defines | `TELEIOS_*` | `TELEIOS_BUILD_DEBUG` |
| Runtime macros | `TL_<NAME>` | `TL_PLATFORM_LINUX` |
| Macro functions | `TL<Name>` | `TLINFO(...)`, `TLFATAL(...)` |
| API functions | `tl_module_function()` | `tl_platform_initialize()` |
| Type aliases | lowercase | `u8`, `i32`, `f32`, `b8` |
| Structs | `TL<Name>` | `TLAllocator`, `TLString` |
| Enums | `TL_<MODULE>_<VALUE>` | `TL_ALLOCATOR_LINEAR` |

**Type System** (`defines.h`):
- Unsigned: `u8`, `u16`, `u32`, `u64`
- Signed: `i8`, `i16`, `i32`, `i64`
- Float: `f32`, `f64` | Boolean: `b8`

**Macros**: `TL_API`, `TL_INLINE`, `TL_NOINLINE`, `TL_THREADLOCAL`, `TL_LIKELY(x)`, `TL_UNLIKELY(x)`

## Architectural Patterns

### Modular .inl Pattern

All complex subsystems use dispatcher pattern with `.inl` files:

```c
// memory.c (dispatcher)
#include "teleios/memory/types.inl"
#include "teleios/memory/linear.inl"
#include "teleios/memory/dynamic.inl"

void* tl_memory_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size) {
    switch (allocator->type) {
        case TL_ALLOCATOR_LINEAR:  return tl_memory_linear_alloc(allocator, tag, size);
        case TL_ALLOCATOR_DYNAMIC: return tl_memory_dynamic_alloc(allocator, tag, size);
        default: TLFATAL("Unsupported type");
    }
}
```

**When to use:**
- Multiple implementations for same interface
- Platform-specific code
- Type-based dispatch at runtime

**Linkage rules:**
- `.inl` functions: `static` by default
- Functions called from other files: non-static (e.g., `tl_window_handler()`)
- Always include `default:` case with `TLFATAL`

### Thread-Safe Container Pattern

Containers dispatch to safe/unsafe variants:
```
container/<type>.inl → <type>_safe.inl / <type>_unsafe.inl
```

All containers accept `thread_safe` parameter in create functions.

### Graphics Threading Model

```
Main Thread                Graphics Thread
-----------                ---------------
• Game logic               • OpenGL context ownership
• glfwPollEvents()        • GLAD initialization
• Submit commands ──────► • Process work queue
```

**Rules:**
1. Main thread NEVER acquires OpenGL context
2. All OpenGL calls via `tl_graphics_submit_sync()` or `tl_graphics_submit_async()`
3. `glfwPollEvents()` on main thread only
4. Sync jobs: stack-allocated args OK
5. Async jobs: heap/static args required

## Global State

The engine uses a single global state struct (`TLGlobal`) initialized in `main.c`:

```c
TLGlobal g = { 0 };
global = &g;
tl_platform_initialize();    // Sets up global allocators
tl_application_initialize(); // Game setup
tl_application_run();        // Main loop
tl_application_terminate();  // Cleanup
tl_platform_terminate();     // Platform cleanup
```

Access via `global->allocator`, `global->scene`, `global->running`, etc.

## Memory System

**LINEAR (Arena Allocator):**
- Fast bump allocation, no individual deallocation
- Auto-grows by ~1.75x when exhausted
- Use for frame/scene-level transient data

**DYNAMIC (Heap Allocator):**
- Individual allocation/deallocation
- Leak detection in Debug builds (with stack traces)
- Warns on dangling allocators at shutdown

**Memory Tags** (17 categories for profiling): `TL_MEMORY_GRAPHICS`, `TL_MEMORY_CONTAINER_*`, `TL_MEMORY_STRING`, `TL_MEMORY_SCENE`, etc.

## Core Modules

| Module | Description |
|--------|-------------|
| **Application** | Fixed timestep game loop with accumulator pattern |
| **Logger** | Levels: VERBOSE, TRACE, DEBUG, INFO, WARN, ERROR, FATAL |
| **Profiler** | Stack-based call tracking (`TL_PROFILER_PUSH/POP`) |
| **Memory** | LINEAR (bump allocator) and DYNAMIC (heap with leak detection) |
| **Platform** | Timing, window management via GLFW |
| **Thread** | Threads, mutexes, condition variables |
| **Graphics** | Dedicated rendering thread with work queue |
| **Event** | Subscriber pattern (`tl_event_subscribe`, `tl_event_submit`) |
| **Container** | Queue, Pool, List, Map, Array, Iterator (all optionally thread-safe) |
| **String** | Immutable `TLString`, `TLStringBuilder` |
| **Scene** | Scene management with Lua scripting and YAML configuration |
| **Config** | YAML configuration parser with scene support |

## Adding New Modules

1. Create `teleios/<module>.h` (public API)
2. Create `teleios/<module>/types.inl`
3. Create `teleios/<module>/<variant>.inl`
4. Create `teleios/<module>.c` (dispatcher)
5. Add to `ENGINE_SOURCES` in `CMakeLists.txt`
6. Include in `teleios/teleios.h`

## Dependencies

- **GLFW 3.4**: Windowing (`<GLFW/glfw3.h>`)
- **libyaml 0.2.5**: YAML parser (`<yaml.h>`)
- **Lua 5.4.7**: Scripting (`<lua.h>`)
- **cglm 0.9.4**: Math (`<cglm/cglm.h>`)
- **GLAD**: OpenGL loader (`<glad/glad.h>`) - vendored
- **STB Image**: Image loading (`<stb/stb_image.h>`) - vendored

## Scene System

Scenes are configured in `application.yml` and use Lua scripts for lifecycle management.

### YAML Configuration

```yaml
application:
  scene:
    - name: main
      script:
        load: scripts/main/load.lua       # REQUIRED
        unload: scripts/main/unload.lua   # REQUIRED
        frame_begin: scripts/main/frame_begin.lua  # REQUIRED
        frame_end: scripts/main/frame_end.lua      # REQUIRED
      graphics:
        clear_color: .7, .2, .7, 1
        depth:
          enabled: true
          function: LEQUAL  # NEVER, LESS, EQUAL, LEQUAL, GREATER, NOTEQUAL, GEQUAL, ALWAYS
        blend:
          enabled: true
          equation: FUNC_ADD  # FUNC_ADD, FUNC_SUBTRACT, FUNC_REVERSE_SUBTRACT, MIN, MAX
          function:
            source: SRC_ALPHA
            target: ONE_MINUS_SRC_ALPHA
      camera:
        property:
          - name: "depth"
            value: "-1.0, 1.0"
```

### Scene Lifecycle

```
activate → init_lua → apply_graphics → apply_camera → load.lua
         ↓
    ┌────────────────────────────────────────────┐
    │                  GAME LOOP                 │
    │  engine clear → frame_begin.lua → step →  │
    │  update → frame_end.lua → swap            │
    └────────────────────────────────────────────┘
         ↓
    unload.lua → close_lua → destroy
```

### Script Responsibilities

| Script | When Called | Purpose |
|--------|-------------|---------|
| `load.lua` | Scene activation | Initialize scene resources |
| `unload.lua` | Scene deactivation | Cleanup scene resources |
| `frame_begin.lua` | After engine frame setup | Client rendering setup |
| `frame_end.lua` | Before swap buffers | UI, debug overlays |

### Engine Fixed Functions (Not Lua)

- `tl_scene_step(dt)` - Fixed timestep physics (future)
- `tl_scene_update(dt)` - Frame update logic (future)
