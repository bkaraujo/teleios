#include "teleios/teleios.h"
#include "teleios/platform_linux.inc"
#include "teleios/platform_windows.inc"
#include "teleios/platform_glfw.inc"
#include <GLFW/glfw3.h>

typedef struct {
    b8   (*initialize       )(void);
    b8   (*terminate        )(void);
    i8   (*path_separator   )(void);
    u64  (*time_epoch_millis)(void);
    u64  (*time_epoch_micros)(void);
    void (*time_clock       )(TLDateTime*);
} TLPlatform;

static TLPlatform platform = { 0 };

b8 tl_platform_initialize(void) {
#if defined(TL_PLATFORM_LINUX)
    platform.initialize         = tl_lnx_platform_initialize;
    platform.path_separator     = tl_lnx_filesystem_path_separator;
    platform.path_separator     = tl_lnx_platform_initialize;
    platform.time_clock         = tl_lnx_time_clock;
    platform.time_epoch_millis  = tl_lnx_time_epoch_millis;
    platform.time_epoch_micros  = tl_lnx_time_epoch_micros;
    platform.terminate          = tl_lnx_platform_terminate;
#else

    platform.initialize         = tl_winapi_platform_initialize;
    platform.path_separator     = tl_winapi_filesystem_path_separator;
    platform.time_clock         = tl_winapi_time_clock;
    platform.time_epoch_millis  = tl_winapi_time_epoch_millis;
    platform.time_epoch_micros  = tl_winapi_time_epoch_micros;
    platform.terminate          = tl_winapi_platform_terminate;
#endif

    if (!platform.initialize()) {
        TLERROR("Platform failed to initialize")
        return false;
    }

    tl_profiler_frame_push("platform.c", 16, "tl_platform_initialize", NULL);

    if (!glfwInit()) {
        TLERROR("GLFW failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }
    
    if (!tl_window_create()) {
        TLERROR("GLFW failed to create Window")
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}

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

b8 tl_platform_terminate(void) {
    TL_PROFILER_PUSH

    glfwTerminate();
    if (!platform.terminate()) {
        TLERROR("Platform failed to terminate")
        return false;
    }

    TL_PROFILER_POP_WITH(true)
}