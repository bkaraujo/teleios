#include "libk/libk.h"

void k_time_clock(KClock* clock) {
#if defined(K_PLATFORM_LINUX)
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
#elif defined(K_PLATFORM_WINDOWS)
    SYSTEMTIME st; GetLocalTime(&st);

    *(u16*)&clock->year   = st.wYear;
    *( u8*)&clock->month  = st.wMonth;
    *( u8*)&clock->day    = st.wDay;
    *( u8*)&clock->hour   = st.wHour;
    *( u8*)&clock->minute = st.wMinute;
    *( u8*)&clock->second = st.wSecond;
    *( u8*)&clock->millis = st.wMilliseconds;
#endif
}

u64 k_time_epoch_millis(void) {
    return k_time_epoch_micros() / 1000;
}

#if defined(K_PLATFORM_LINUX)
#	include <sys/time.h>
#endif

u64 k_time_epoch_micros(void) {
    u64 micros = 0;
#if defined(K_PLATFORM_LINUX)
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    micros = (uint64_t) now.tv_sec * 1000000 + now.tv_usec;
#elif defined(K_PLATFORM_WINDOWS)
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