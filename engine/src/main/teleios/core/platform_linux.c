#include "teleios/core/platform_detector.h"
#ifdef TLPLATFORM_LINUX
#include "teleios/core/logger.h"

// ########################################################
//                    TIME FUNCTIONS
// ########################################################
#define __USE_POSIX 1
#define __USE_POSIX199309 1
#include <time.h>
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

#endif