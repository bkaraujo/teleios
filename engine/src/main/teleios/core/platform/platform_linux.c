#include "teleios/defines.h"
#ifdef TLPLATFORM_LINUX
#include "teleios/core.h"

// ########################################################
//                    MEMORY FUNCTIONS
// ########################################################
#include "teleios/core/platform/memory.h"

void* tl_platform_memory_alloc(const u64 size) {
    return malloc(size);
}

void tl_platform_memory_free(void *block) {
    free(block);
}
void tl_platform_memory_set(void *block, const i32 value, const u64 size) {
    memset(block, value, size);
}

void tl_platform_memory_copy(void *target, const void *source, const u64 size) {
    memcpy(target, source, size);
}

// ########################################################
//                    TIME FUNCTIONS
// ########################################################
#include <time.h>
#include <sys/time.h>

void tl_time_clock(TLClock* clock) {
    TLSTACKPUSHA("0x%p", clock)
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME_COARSE, &now) != 0) TLSTACKPOP

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) TLSTACKPOP

    clock->year = localtime.tm_year + 1900;
    clock->month = localtime.tm_mon + 1;
    clock->day = localtime.tm_mday;
    clock->hour = localtime.tm_hour;
    clock->minute = localtime.tm_min;
    clock->second = localtime.tm_sec;
    clock->millis = now.tv_nsec / 1000;

    TLSTACKPOP
}

u64 tl_time_epoch_millis(void) {
    TLSTACKPUSH
    struct timespec now = { 0 };
    clock_gettime(CLOCK_REALTIME_COARSE, &now);
    const u64 micros = (uint64_t) now.tv_sec * 1000000 + now.tv_nsec;
    TLSTACKPOPV(micros);
}

u64 tl_time_epoch_micros(void) {
    TLSTACKPUSH
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    const u64 micros = (uint64_t) now.tv_sec * 1000000 + now.tv_usec;
    TLSTACKPOPV(micros);
}
#endif