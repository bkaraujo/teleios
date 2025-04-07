#include "bks/bks.h"
#include <time.h>

#if defined(BKS_PLATFORM_LINUX)
#	include <sys/time.h>
#endif
// #####################################################################################################################
//
//                                                     TIME
//
// #####################################################################################################################
void bks_time_clock(BKSClock* clock) {
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

u64 bks_time_epoch_millis(void) {
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);
    return (uint64_t) now.tv_sec * 1000000 + now.tv_nsec;
}

u64 bks_time_epoch_micros(void) {
    u64 micros = 0;
#if defined(BKS_PLATFORM_LINUX)
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    micros = (uint64_t) now.tv_sec * 1000000 + now.tv_usec;
#elif defined(BKS_PLATFORM_WINDOWS)
    FILETIME ft; GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // FILETIME is in 100-nanosecond intervals since January 1, 1601 (UTC).
    // We need to convert it to microseconds since January 1, 1970 (UTC).

    // Subtract the number of 100-nanosecond intervals between the two dates.
    // The value is 116444736000000000 (obtained from various sources).
    micros = (uli.QuadPart - 116444736000000000ULL) / 10; // Convert to microseconds
#endif
    return micros;
}