#include "teleios/platform_linux.inc"
#include "teleios/platform_windows.inc"
#include <GLFW/glfw3.h>

typedef struct {
    b8   (*terminate        )(void);
    i8   (*path_separator   )(void);
    u64  (*time_epoch_millis)(void);
    u64  (*time_epoch_micros)(void);
    void (*time_clock       )(TLDateTime*);
} TLPlatform;

static TLPlatform platform = { 0 };

b8 tl_platform_initialize(void) {
#if defined(TL_PLATFORM_LINUX)
    if (!tl_lnx_platform_initialize()) {
        TLERROR("Platform failed to initialize")
        return false;
    }

    platform.path_separator     = tl_lnx_platform_initialize;
    platform.time_clock         = tl_lnx_time_clock;
    platform.time_epoch_millis  = tl_lnx_time_epoch_millis;
    platform.time_epoch_micros  = tl_lnx_time_epoch_micros;
    platform.terminate          = tl_lnx_platform_terminate;
#else
    if (!tl_winapi_platform_initialize()) {
        TLERROR("Platform failed to initialize")
        return false;
    }

    platform.path_separator     = tl_winapi_filesystem_path_separator;
    platform.time_clock         = tl_winapi_time_clock;
    platform.time_epoch_millis  = tl_winapi_time_epoch_millis;
    platform.time_epoch_micros  = tl_winapi_time_epoch_micros;
    platform.terminate          = tl_winapi_platform_terminate;
#endif

    if (!glfwInit()) {
        TLERROR("GLFW failed to initialize")
        return false;
    }

    return true;
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
    glfwTerminate();

    if (!platform.terminate()) {
        TLERROR("Platform failed to terminate")
        return false;
    }

    return true;
}