#ifndef __TELEIOS_PLATFORM_LINUX__
#define __TELEIOS_PLATFORM_LINUX__

#include "teleios/defines.h"

#ifdef TL_PLATFORM_LINUX

#include "teleios/teleios.h"
#include <time.h>

// ---------------------------------
// Linux Platform - Initialization
// ---------------------------------

static b8 tl_lnx_initialize(void) {
    TL_PROFILER_PUSH
    // Linux-specific platform initialization
    // (Currently no specific setup needed)
    TL_PROFILER_POP_WITH(true)
}

static b8 tl_lnx_terminate(void) {
    TL_PROFILER_PUSH
    // Linux-specific platform cleanup
    // (Currently no specific cleanup needed)
    TL_PROFILER_POP_WITH(true)
}

// ---------------------------------
// Linux Platform - Filesystem
// ---------------------------------

static i8 tl_lnx_filesystem_path_separator(void) {
    return '/';
}

// ---------------------------------
// Linux Platform - Timing
// ---------------------------------

static void tl_lnx_time_clock(TLDateTime* clock) {
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    clock->year = localtime.tm_year + 1900;
    clock->month = (u8)(localtime.tm_mon + 1);
    clock->day = (u8)localtime.tm_mday;
    clock->hour = (u8)localtime.tm_hour;
    clock->minute = (u8)localtime.tm_min;
    clock->second = (u8)localtime.tm_sec;
    clock->millis = (u16)(now.tv_nsec / 1000000);
}

static u64 tl_lnx_time_epoch_millis(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);  // ~20-30 ns
    return (u64)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static u64 tl_lnx_time_epoch_micros(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME, &now);  // ~25-50 ns
    return (u64)now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

#endif // TL_PLATFORM_LINUX

#endif // __TELEIOS_PLATFORM_LINUX__