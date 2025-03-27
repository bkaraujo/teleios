#include "teleios/core.h"
#include "teleios/globals.h"
#include "teleios/core/platform/memory.h"

#if defined(TLPLATFORM_LINUX)
#include <time.h>
#include <sys/time.h>
#endif

typedef struct {
    const char* name;
    u64 timestamp;
    u64 ticks;
} TLProfile;

static TLProfile profile[U8_MAX];

void tl_profiler_begin(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    u32 index = 0;
    for (u32 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) {
            index = i;
            break;
        }
    }

    profile[index].name = name;
#if defined(TLPLATFORM_LINUX)
    struct timeval now; gettimeofday(&now, NULL);
    profile[index].timestamp = (u64) now.tv_sec * 1000000 + now.tv_usec;
#elif defined(TLPLATFORM_WINDOWS)
#erro "!!!"
#endif
#endif
}

static u8 tl_profiler_index(const char *name) {
    for (u8 i = 0; i < U8_MAX; ++i) {
        if (profile[i].name == NULL) continue;

        b8 found = TRUE;
        for (u64 j = 0; j < U64_MAX; ++j) {
            if (profile[i].name[j] != name[j]) {
                found = FALSE;
                break;
            }

            if (name[j] == '\0') break;
        }

        if (found) return i;
    }

    TLFATAL("Profile [%s] not found", name)
}

u64 tl_profiler_time(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
#if defined(TLPLATFORM_LINUX)
    struct timeval now = { 0 };
    gettimeofday(&now, NULL);
    return (uint64_t) now.tv_sec * 1000000 + now.tv_usec - profile[index].timestamp;
#elif defined(TLPLATFORM_WINDOWS)
#erro "!!!"
#else
    return U64_MAX;
#endif
#endif
}

void tl_profiler_tick(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    profile[index].ticks++;
#endif
}

u64 tl_profiler_ticks(const char *name) {
#if ! defined(TELEIOS_BUILD_RELEASE)
    const u8 index = tl_profiler_index(name);
    return profile[index].ticks;
#else
    return U64_MAX;
#endif
}

void tl_profiler_end(const char *name) {
    const u8 index = tl_profiler_index(name);
    tl_platform_memory_set(&profile[index], 0, sizeof(TLProfile));
}