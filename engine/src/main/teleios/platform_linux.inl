#include "teleios/defines.h"
#if defined(K_PLATFORM_LINUX)
#include "teleios/teleios.h"

i8 tl_lnx_filesystem_path_separator(void) {
    return '/';
}

void tl_lnx_time_clock(TLDateTIme* clock) {
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    clock->year = localtime.tm_year + 1900;
    clock->month = localtime.tm_mon + 1;
    clock->day = localtime.tm_mday;
    clock->hour = localtime.tm_hour;
    clock->minute = localtime.tm_min;
    clock->second = localtime.tm_sec;
    clock->millis = now.tv_nsec / 1000;
}

u64 tl_lnx_time_epoch_millis(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);                     // ~20-30 ns
    return (uint64_t) now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

u64 tl_lnx_time_epoch_micros(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME, &now);                            // ~25-50 ns
    return (uint64_t) now.tv_sec * 1000000 + now.tv_nsec / 1000;
}
#endif