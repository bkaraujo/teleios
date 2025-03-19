#include "teleios/core.h"

typedef struct {
    TLString* name;
    u64 timestamp;
    u64 ticks;
} TLProfile;

TLList* profilers = NULL;

void tl_profiler_begin(const char *name) {
    TLSTACKPUSHA("%s", name)

#ifndef TELEIOS_BUILD_RELEASE
    if (profilers == NULL) {
        profilers = tl_list_create(runtime->arenas.permanent);
    }

    TLProfile* profile = tl_memory_alloc(runtime->arenas.permanent, sizeof(TLProfile), TL_MEMORY_PROFILER);
    profile->name = tl_string_wrap(runtime->arenas.permanent, name);
    profile->timestamp = tl_time_epoch();
    tl_list_add(profilers, profile);
#endif
    TLSTACKPOP
}

TLProfile* tl_profiler_current(const char *name) {
    TLSTACKPUSHA("0x%p", name)
    TLIterator* it = tl_list_iterator_create(profilers);
    TLProfile* profile = NULL;
    for (profile = tl_list_iterator_next(it) ; profile != NULL ; profile = tl_list_iterator_next(it)) {
        if (tl_string_equals(profile->name, name)) {
            TLSTACKPOP
            return profile;
        }
    }

    TLSTACKPOP
    return NULL;
}

void tl_profiler_tick(const char *name) {
    TLSTACKPUSHA("%s", name)
#ifndef TELEIOS_BUILD_RELEASE
    TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) { profile->ticks++; }
#endif
    TLSTACKPOP
}

u64 tl_profiler_time(const char *name) {
    TLSTACKPUSHA("%s", name)
#ifndef TELEIOS_BUILD_RELEASE
    const TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) {
        TLSTACKPOP
        return tl_time_epoch() - profile->timestamp;
    }
#endif
    TLSTACKPOP
    return U64_MAX;
}

u64 tl_profiler_count(const char *name) {
    TLSTACKPUSHA("%s", name)
#ifndef TELEIOS_BUILD_RELEASE
    const TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) {
        TLSTACKPOP
        return profile->ticks;
    }
#endif
    TLSTACKPOP
    return U64_MAX;
}

void tl_profiler_end(const char *name) {
    TLSTACKPUSHA("%s", name)
#ifndef TELEIOS_BUILD_RELEASE
    TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) { tl_list_remove(profilers, profile); }
#endif
    TLSTACKPOP
}