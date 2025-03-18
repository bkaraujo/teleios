#include "teleios/core.h"

typedef struct {
    TLString* name;
    u64 timestamp;
    u64 ticks;
} TLProfile;

TLList* profilers = NULL;

void tl_profiler_begin(const char *name) {
#ifdef TELEIOS_BUILD_PROFILE
    if (profilers == NULL) {
        profilers = tl_list_create(runtime->arenas.permanent);
    }

    TLProfile* profile = tl_memory_alloc(runtime->arenas.permanent, sizeof(TLProfile), TL_MEMORY_PROFILER);
    profile->name = tl_string_wrap(runtime->arenas.permanent, name);
    profile->timestamp = tl_time_epoch();
    tl_list_add(profilers, profile);
#endif
}

TLProfile* tl_profiler_current(const char *name) {
    TLIterator* it = tl_list_iterator_create(profilers);
    TLProfile* profile = NULL;
    for (profile = tl_list_iterator_next(it) ; profile != NULL ; profile = tl_list_iterator_next(it)) {
        if (tl_string_equals(profile->name, name)) {
            return profile;
        }
    }

    return NULL;
}

void tl_profiler_tick(const char *name) {
#ifdef TELEIOS_BUILD_PROFILE
    TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) { profile->ticks++; }
#endif
}

u64 tl_profiler_time(const char *name) {
#ifdef TELEIOS_BUILD_PROFILE
    const TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) { return tl_time_epoch() - profile->timestamp; }
#endif
    return U64_MAX;
}

u64 tl_profiler_count(const char *name) {
#ifdef TELEIOS_BUILD_PROFILE
    const TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) { return profile->ticks; }
#endif
    return U64_MAX;
}

void tl_profiler_end(const char *name) {
#ifdef TELEIOS_BUILD_PROFILE
    TLProfile* profile = tl_profiler_current(name);
    if (profile != NULL) { tl_list_remove(profilers, profile); }
#endif
}