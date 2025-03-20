#include "teleios/defines.h"
#ifdef TLPLATFORM_LINUX
#include "teleios/core.h"

// ########################################################
//                    TIME FUNCTIONS
// ########################################################
#include <time.h>
#include <sys/time.h>
#include "teleios/core/time.h"

void tl_time_clock(TLClock* clock) {
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

u64 tl_time_epoch(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    const u64 micros = (uint64_t) tv.tv_sec * 1000000 + tv.tv_usec;

    return micros;
}

#endif