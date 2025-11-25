#include "teleios/teleios.h"
#include "teleios/platform/types.inl"
#include "teleios/platform/windows.inl"
#include "teleios/platform/linux.inl"
#include "teleios/platform/glfw.inl"
#include <GLFW/glfw3.h>

#include "teleios/graphics.h"
#include "teleios/simulation.h"

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
    platform.initialize             = tl_lnx_initialize;
    platform.terminate              = tl_lnx_terminate;
    platform.path_separator         = tl_lnx_filesystem_path_separator;
    platform.get_current_directory  = tl_lnx_filesystem_get_current_directory;
    platform.time_clock             = tl_lnx_time_clock;
    platform.time_epoch_millis      = tl_lnx_time_epoch_millis;
    platform.time_epoch_micros      = tl_lnx_time_epoch_micros;
#else
    platform.initialize             = tl_winapi_initialize;
    platform.terminate              = tl_winapi_terminate;
    platform.path_separator         = tl_winapi_filesystem_path_separator;
    platform.get_current_directory  = tl_winapi_filesystem_get_current_directory;
    platform.time_clock             = tl_winapi_time_clock;
    platform.time_epoch_millis      = tl_winapi_time_epoch_millis;
    platform.time_epoch_micros      = tl_winapi_time_epoch_micros;
#endif
    TL_PROFILER_PUSH

    TLINFO("Iniciando %s", platform.get_current_directory())

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

    if (!tl_config_initialize()) {
        TLERROR("Config system failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    // Initialize GLFW
    TLDEBUG("GLFW %s", glfwGetVersionString())
    if (!glfwInit()) {
        TLERROR("GLFW failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_input_initialize()) {
        TLERROR("Input system failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    // Create window
    if (!tl_window_create()) {
        TLERROR("GLFW failed to create window")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_simulation_initialize()) {
        TLERROR("Simulation failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

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
 * Graphics → Simulation → Input → Window → GLFW → Config → Memory → Platform
 */
b8 tl_platform_terminate(void) {
    TL_PROFILER_PUSH

    // Terminate window
    tl_window_terminate();

    if (!tl_input_terminate()) {
        TLERROR("Input system failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_graphics_terminate()) {
        TLERROR("Graphics failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_simulation_terminate()) {
        TLERROR("Simulation failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    // Terminate GLFW
    glfwTerminate();

    if (!tl_config_terminate()) {
        TLERROR("Config system failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    // Terminate memory system (last, as other systems may use it)
    if (!tl_memory_terminate()) {
        TLERROR("Memory system failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    // Terminate platform-specific subsystem
    if (!platform.terminate()) {
        TLERROR("Platform failed to terminate")
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