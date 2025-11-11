#include "teleios/teleios.h"
#include "teleios/platform_types.inl"
#include "teleios/platform_windows.inl"
#include "teleios/platform_linux.inl"
#include "teleios/platform_glfw.inl"
#include <GLFW/glfw3.h>

/**
 * @brief Platform dispatcher using function pointer table
 *
 * IMPORTANT: Platform uses compile-time dispatch via #ifdef guards,
 * not runtime dispatch like the memory system. This is because:
 * 1. Platform is known at compile time
 * 2. No runtime overhead for platform detection
 * 3. Dead code elimination removes unused platform code
 *
 * Pattern: Compile-time selection → Function pointers → Dispatcher calls
 * This differs from memory.c which uses: Runtime enum → switch/case → Implementation
 */
static TLPlatform platform = { 0 };

/**
 * @brief Initialize platform layer
 *
 * This function:
 * 1. Initializes platform-specific subsystem (timing, etc)
 * 2. Initializes cross-cutting concerns (memory, GLFW, window, graphics)
 *
 * NOTE: This mixing of concerns is intentional - platform initialization
 * is the entry point for the entire engine.
 */
b8 tl_platform_initialize(void) {
#ifdef TL_PLATFORM_LINUX
    platform.initialize         = tl_lnx_initialize;
    platform.terminate          = tl_lnx_terminate;
    platform.path_separator     = tl_lnx_filesystem_path_separator;
    platform.time_clock         = tl_lnx_time_clock;
    platform.time_epoch_millis  = tl_lnx_time_epoch_millis;
    platform.time_epoch_micros  = tl_lnx_time_epoch_micros;
#else
    platform.initialize         = tl_winapi_initialize;
    platform.terminate          = tl_winapi_terminate;
    platform.path_separator     = tl_winapi_filesystem_path_separator;
    platform.time_clock         = tl_winapi_time_clock;
    platform.time_epoch_millis  = tl_winapi_time_epoch_millis;
    platform.time_epoch_micros  = tl_winapi_time_epoch_micros;
#endif

    // NOW safe to use profiler/logger since time functions are initialized
    TL_PROFILER_PUSH

    // Initialize platform-specific subsystem
    if (!platform.initialize()) {
        TLERROR("Platform failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    // Initialize memory system
    if (!tl_memory_initialize()) {
        TLERROR("Memory system failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    // Initialize GLFW
    if (!glfwInit()) {
        TLERROR("GLFW failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    // Create window
    if (!tl_window_create()) {
        TLERROR("GLFW failed to create window")
        TL_PROFILER_POP_WITH(false)
    }

    // Initialize graphics system
    if (!tl_graphics_initialize()) {
        TLERROR("Graphics failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}

/**
 * @brief Terminate platform layer
 *
 * Cleanup order is reverse of initialization:
 * Graphics → Window → GLFW → Platform → Memory
 */
b8 tl_platform_terminate(void) {
    TL_PROFILER_PUSH

    // Terminate graphics
    if (!tl_graphics_terminate()) {
        TLERROR("Graphics failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    // Terminate window
    tl_window_terminate();

    // Terminate GLFW
    glfwTerminate();

    // Terminate platform-specific subsystem
    if (!platform.terminate()) {
        TLERROR("Platform failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    // Terminate memory system (last, as other systems may use it)
    if (!tl_memory_terminate()) {
        TLERROR("Memory system failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}

// ---------------------------------
// Platform API Dispatchers
// ---------------------------------

i8 tl_filesystem_path_separator(void) {
    return platform.path_separator();
}

void tl_time_clock(TLDateTime* clock) {
    platform.time_clock(clock);
}

u64 tl_time_epoch_millis(void) {
    return platform.time_epoch_millis();
}

u64 tl_time_epoch_micros(void) {
    return platform.time_epoch_micros();
}