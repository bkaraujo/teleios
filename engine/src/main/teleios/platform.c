#include "teleios/platform_linux.inc"
#include "teleios/platform_windows.inc"
#include <GLFW/glfw3.h>

b8 tl_platform_initialize(void) {
#if defined(TL_PLATFORM_LINUX)
    return tl_lnx_platform_initialize();
#else
    if (!tl_winapi_platform_initialize()) {
        TLERROR("Platform failed to initialize")
        return false;
    }
#endif

    if (!glfwInit()) {
        TLERROR("GLFW failed to initialize")
        return false;
    }

    return true;
}

char tl_filesystem_path_separator() {
#if defined(TL_PLATFORM_LINUX)
    return tl_lnx_filesystem_path_separator();
#else
    return tl_winapi_filesystem_path_separator();
#endif
}

void tl_time_clock(TLDateTime* clock) {
#if defined(TL_PLATFORM_LINUX)
    tl_lnx_time_clock(clock);
#else
    tl_winapi_time_clock(clock);
#endif
}

u64 tl_time_epoch_millis(void) {
#if defined(TL_PLATFORM_LINUX)
    return tl_lnx_time_epoch_millis();
#else
    return tl_winapi_time_epoch_millis();
#endif
}

u64 tl_time_epoch_micros(void) {
#if defined(TL_PLATFORM_LINUX)
    return tl_lnx_time_epoch_micros();
#else
    return tl_winapi_time_epoch_micros();
#endif
}

b8 tl_platform_terminate(void) {
#if defined(TL_PLATFORM_LINUX)
    return tl_lnx_platform_terminate();
#else
    return tl_winapi_platform_terminate();
#endif
}