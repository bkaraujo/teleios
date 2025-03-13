#include "teleios/platform_detector.h"
#ifdef TLPLATFORM_LINUX

#include <stdio.h>
#include <malloc.h>

#define SECS_IN_DAY (24 * 60 * 60)
// ########################################################
//                     CONSOLE FUNCTIONS
// ########################################################
#include "teleios/console.h"
void tl_console_stdout(TLLogLevel level, const char *message) {
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE, VERBOSE
    static const char *strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30", "1;29"};
    fprintf(stdout, "\033[%sm%s\033[0m", strings[level], message);
}
// ########################################################
//                    TIME FUNCTIONS
// ########################################################
#define __USE_POSIX 1
#define __USE_POSIX199309 1
#include <time.h>
#include "teleios/time.h"

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
// ########################################################
//                    MEMORY FUNCTIONS
// ########################################################
#include "teleios/memory.h"

typedef struct {
    u64 index;
    char *payload;
} TLMemoryPage;

struct TLMemoryArena {
    u64 allocated;
    u64 tagged_count[TL_MEMORY_MAXIMUM];
    u64 tagged_size[TL_MEMORY_MAXIMUM];
    u64 page_size;
    TLMemoryPage page[U8_MAX];
};

static const char *tl_memory_name(TLMemoryTag tag) {
    switch (tag) {
        case TL_MEMORY_BLOCK: return "TL_MEMORY_BLOCK";
        case TL_MEMORY_CONTAINER_LIST: return "TL_MEMORY_CONTAINER_LIST";
        case TL_MEMORY_CONTAINER_NODE: return "TL_MEMORY_CONTAINER_NODE";
        case TL_MEMORY_WINDOW: return "TL_MEMORY_WINDOW";
        case TL_MEMORY_MAXIMUM: return "TL_MEMORY_MAXIMUM";
    }

    return "???";
}

static TLMemoryArena* arenas[U8_MAX] = { 0 };

TLMemoryArena* tl_memory_arena_create(u64 size) {
    TLTRACE(">> tl_memory_arena_create(%d)", size)
    // ----------------------------------------------------------
    // Create the memory arena
    // ----------------------------------------------------------
    TLMemoryArena* arena = __builtin_malloc(sizeof(TLMemoryArena));
    if (arena == NULL) TLFATAL("Failed to allocate TLMemoryArena");
    __builtin_memset(arena, 0, sizeof(TLMemoryArena));
    arena->page_size = size;
    // ----------------------------------------------------------
    // Keep track of the created arena
    // ----------------------------------------------------------
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] == NULL) {
            arenas[i] = arena;
            break;
        }
    }

    TLTRACE("Created TLMemoryArena 0x%p", arena)
    TLVERBOSE("TLMemoryArena 0x%p page size: %d bytes", arena, arena->page_size)
    
    TLTRACE("<< tl_memory_arena_create(%d)", size)
    return arena;
}

TLINLINE static b8 __tl_memory_arena_is_valid(TLMemoryArena* arena) {
    TLTRACE(">> __tl_memory_arena_is_valid(0x%p)", arena)

    if (arena == NULL) {
        TLWARN("TLMemoryArena is NULL")
        TLTRACE("<< __tl_memory_arena_is_valid(0x%p)", arena)
        return FALSE;
    }

    b8 found = FALSE;
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] == arena) {
            found = TRUE;
            arenas[i] = NULL;
        }
    }

    if (!found) {
        TLWARN("Illegal TLMemoryArena 0x%p", arena)
    }

    TLTRACE("<< __tl_memory_arena_is_valid(0x%p)", arena)
    return found;
}


TLINLINE static void __tl_memory_arena_destroy(TLMemoryArena* arena) {
    TLTRACE(">> __tl_memory_arena_destroy(0x%p)", arena)
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arena->page[i].payload != NULL) {
            TLVERBOSE("Releasing page %d", i)
            __builtin_free(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (arena->tagged_size[i] != 0) {
            TLVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    __builtin_free(arena);
    TLTRACE("<< __tl_memory_arena_destroy(0x%p)", arena)
    arena = NULL;
}

void tl_memory_arena_destroy(TLMemoryArena* arena) {
    TLTRACE(">> tl_memory_arena_destroy(0x%p)", arena)
    TLTRACE("Destroying TLMemoryArena 0x%p", arena)
    
    if (!__tl_memory_arena_is_valid(arena)) {
        TLTRACE("<< tl_memory_arena_destroy(0x%p)", arena)
        return;
    }

    __tl_memory_arena_destroy(arena);
    TLTRACE("<< tl_memory_arena_destroy(0x%p)", arena)
}

void *tl_memory_alloc(TLMemoryArena* arena, u64 size, TLMemoryTag tag) {
    TLTRACE(">> tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (!__tl_memory_arena_is_valid(arena)) {
        TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
        return NULL;
    }

    if (size == 0) {
        TLWARN("Allocation size must be greater then 0")
        TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
        return NULL;
    }

    if (size > arena->page_size) {
        TLWARN("TLMemoryArena with page size of %d bytes. It cannot allocate %d bytes", arena->page_size, size)
        TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
        return NULL;
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {
        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            TLVERBOSE("Initializing page %d", i)
            arena->page[i].payload = __builtin_malloc(arena->page_size);
            
            found = i;
            break;
        }
        
        // Utilize the available TLMemoryPage
        if (arena->page[i].index + size <= arena->page_size) {
            TLVERBOSE("Using page %d", i)
            
            found = i;
            break;
        }
    }
    
    if (found == U8_MAX) {
        TLWARN("No suitable TLMemoryPage within the arena")
        return NULL;
    }
    // -------------------------------------------------
    // Adjust the TLMemoryArena internal state
    // -------------------------------------------------
    arena->allocated += size;
    arena->tagged_count[tag] += 1;
    arena->tagged_size[tag] += size;
    // -------------------------------------------------
    // Adjust the TLMemoryPage internal state
    // -------------------------------------------------
    void* address = arena->page[found].payload + arena->page[found].index;
    arena->page[found].index += size;
    TLVERBOSE("Page %d has %d bytes available", found, (arena->page_size - arena->page[found].index))
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    TLTRACE("<< tl_memory_alloc(0x%p, %d, %s)", arena, size, tl_memory_name(tag))
    return address;
}

void tl_memory_set(void *block, i32 value, u64 size) {
    TLTRACE(">> tl_memory_set(0x%p, %d, %llu)", block, value, size);
    __builtin_memset(block, value, size);
    TLTRACE("<< tl_memory_set(0x%p, %d, %llu)", block, value, size);
}

void tl_memory_copy(void *target, void *source, u64 size) {
    TLTRACE(">> tl_memory_copy(0x%p, 0x%p, %d)", target, source, size);
    __builtin_memcpy(target, source, size);
    TLTRACE("<< tl_memory_copy(0x%p, 0x%p, %d)", target, source, size);
}
// ########################################################
//                  LIFECYCLE FUNCTIONS
// ########################################################
b8 tl_platform_initialize(void) {
    TLTRACE(">> tl_platform_initialize(void)")

    TLTRACE("<< tl_platform_initialize(void)")
    return TRUE;
}

b8 tl_platform_terminate(void) {
    TLTRACE(">> tl_platform_terminate(void)")
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] != NULL) {
            TLWARN("Removing dangling TLMemoryArena 0x%p", arenas[i])
            __tl_memory_arena_destroy(arenas[i]);
        }
    }

    TLTRACE("<< tl_platform_terminate(void)")
    return TRUE;
}

#endif