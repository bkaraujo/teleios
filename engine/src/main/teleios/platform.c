#include "teleios/teleios.h"
#include "teleios/platform/types.inl"
#include "teleios/platform/windows.inl"
#include "teleios/platform/linux.inl"
#include "teleios/platform/glfw.inl"
#include <GLFW/glfw3.h>

#include "teleios/graphics.h"

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
    platform.time_clock             = tl_lnx_time_clock;
    platform.time_epoch_millis      = tl_lnx_time_epoch_millis;
    platform.time_epoch_micros      = tl_lnx_time_epoch_micros;
    platform.fs_read                = tl_lnx_filesystem_read;
    platform.fs_size                = tl_lnx_filesystem_size;
    platform.fs_exists              = tl_lnx_filesystem_exists;
    platform.fs_path_separator      = tl_lnx_filesystem_path_separator;
    platform.fs_current_directory   = tl_lnx_filesystem_get_current_directory;
#else
    platform.initialize             = tl_winapi_initialize;
    platform.terminate              = tl_winapi_terminate;
    platform.time_clock             = tl_winapi_time_clock;
    platform.time_epoch_millis      = tl_winapi_time_epoch_millis;
    platform.time_epoch_micros      = tl_winapi_time_epoch_micros;
    platform.fs_read                = tl_winapi_filesystem_read;
    platform.fs_size                = tl_winapi_filesystem_size;
    platform.fs_exists              = tl_winapi_filesystem_exists;
    platform.fs_path_separator      = tl_winapi_filesystem_path_separator;
    platform.fs_current_directory   = tl_winapi_filesystem_get_current_directory;
#endif
    TL_PROFILER_PUSH

    if (!platform.initialize()) {
        TLERROR("Platform failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_memory_initialize()) {
        TLERROR("Memory system failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_config_initialize()) {
        TLERROR("Config system failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    TLDEBUG("GLFW %s", glfwGetVersionString())
    if (!glfwInit()) {
        TLERROR("GLFW failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_window_create()) {
        TLERROR("GLFW failed to create window")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_graphics_initialize()) {
        TLERROR("Graphics failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_input_initialize()) {
        TLERROR("Input system failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}

b8 tl_platform_terminate(void) {
    TL_PROFILER_PUSH

    if (!tl_graphics_terminate()) {
        TLERROR("Graphics failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    tl_window_terminate();
    glfwTerminate();

    if (!tl_config_terminate()) {
        TLERROR("Config system failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_memory_terminate()) {
        TLERROR("Memory system failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    if (!platform.terminate()) {
        TLERROR("Platform failed to terminate")
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}

// ---------------------------------
// Time API Dispatchers
// ---------------------------------

void tl_time_clock(TLDateTime* clock) {
    platform.time_clock(clock);
}

u64 tl_time_epoch_millis(void) {
    return platform.time_epoch_millis();
}

u64 tl_time_epoch_micros(void) {
    return platform.time_epoch_micros();
}

// ---------------------------------
// Filesystem API Dispatchers
// ---------------------------------

i8 tl_filesystem_path_separator(void) {
    return platform.fs_path_separator();
}

const char* tl_filesystem_get_current_directory(void) {
    return platform.fs_current_directory();
}

TLString* tl_filesystem_read(const TLString* path) {
    if (path == NULL) return NULL;
    return platform.fs_read(path);
}

b8 tl_filesystem_exists(const TLString* path) {
    if (path == NULL) return false;
    return platform.fs_exists(path);
}

u64 tl_filesystem_size(const TLString* path) {
    if (path == NULL) return 0;
    return platform.fs_size(path);
}