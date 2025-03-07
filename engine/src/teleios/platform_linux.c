#include "teleios/platform.h"
#ifdef TLPLATFORM_LINUX

#define __USE_POSIX 1
#define __USE_POSIX199309 1
#include <time.h>
#include <stdio.h>
#include <malloc.h>

#define SECS_IN_DAY (24 * 60 * 60)
// ########################################################
//                     CONSOLE FUNCTIONS
// ########################################################
void tl_platform_stdout(TLLogLevel level, const char* message) {
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE, VERBOSE
    static const char* strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30", "1;29"};
    fprintf(stdout, "\033[%sm%s\033[0m", strings[level], message);
}
// ########################################################
//                    TIME FUNCTIONS
// ########################################################
void tl_platform_clock(TLClock* clock) {
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
// 
// __builtin_malloc - GCC is free to eliminate calls if its result is never used, because there are no additional side-effects.
// __builtin_free   - GCC is free to eliminate calls if its result is never used, because there are no additional side-effects.
// ########################################################
typedef struct {
    u64 allocated;
    u64 tagged_count[TL_MEMORY_MAXIMUM];
    u64 tagged_size[TL_MEMORY_MAXIMUM];
} TLMemoryRegistry;

typedef struct {
    u64 size;           // 8 bytes
    TLMemoryTag tag;    // 4 bytes
    void* payload;      // 8 bytes
} TLMemoryBlock;

static TLMemoryRegistry* m_memory_registry;

static const char* tl_platform_memory_name(TLMemoryTag tag) {
    switch (tag) {
        case TL_MEMORY_BLOCK: return "TL_MEMORY_BLOCK";
        case TL_MEMORY_CONTAINER_LIST: return "TL_MEMORY_CONTAINER_LIST";
        case TL_MEMORY_CONTAINER_NODE: return "TL_MEMORY_CONTAINER_NODE";
        case TL_MEMORY_MAXIMUM: return "TL_MEMORY_MAXIMUM";
    }

    return "???";
}

void* tl_platform_memory_alloc(u64 size, TLMemoryTag tag) {
    TLVERBOSE("tl_platform_memory_alloc(%d, %s)", size, tl_platform_memory_name(tag));
    TLMemoryBlock* block = (TLMemoryBlock*) __builtin_malloc(sizeof(TLMemoryBlock));
    if (block == NULL) { return NULL; }

    m_memory_registry->allocated += sizeof(TLMemoryBlock);
    m_memory_registry->tagged_count[TL_MEMORY_BLOCK] += 1;
    m_memory_registry->tagged_size[TL_MEMORY_BLOCK] += sizeof(TLMemoryBlock);

    block->size = size;
    block->tag = tag;
    block->payload = __builtin_malloc(size);

    if (block->payload == NULL) {
        tl_platform_memory_free(block);
        return NULL;
    }

    m_memory_registry->allocated += block->size;
    m_memory_registry->tagged_count[block->tag] += 1;
    m_memory_registry->tagged_size[block->tag] += block->size;

    return block->payload;
}

void tl_platform_memory_free(void* address) {
    TLVERBOSE("tl_platform_memory_free(0x%p)", address);
    TLMemoryBlock* block = (TLMemoryBlock*) (((u64*) address) - 4);
    
    if (block->payload != NULL) {
        __builtin_free((void*) block->payload);
        m_memory_registry->allocated -= block->size;
        m_memory_registry->tagged_count[block->tag] -= 1;
        m_memory_registry->tagged_size[block->tag] -= block->size;
    }

    __builtin_free((void*) block);
    m_memory_registry->allocated -= sizeof(TLMemoryBlock);
    m_memory_registry->tagged_count[TL_MEMORY_BLOCK] -= 1;
    m_memory_registry->tagged_size[TL_MEMORY_BLOCK] -= sizeof(TLMemoryBlock);
}

void tl_platform_memory_set(void* block, i32 value, u64 size) {
    TLVERBOSE("tl_platform_memory_set(0x%p, %d, %llu)", block, value, size);
    __builtin_memset(block, value, size);
}

void tl_platform_memory_copy(void* target, void* source, u64 size) {
    TLVERBOSE("tl_platform_memory_copy(0x%p, 0x%p, %d)", target, source, size);
    __builtin_memcpy(target, source, size);
}
// ########################################################
//                  LIFECYCLE FUNCTIONS
// ########################################################
b8 tl_platform_initialize(void) {
    TLTRACE("tl_platform_initialize(void)")
    m_memory_registry = (TLMemoryRegistry*) __builtin_malloc(sizeof(TLMemoryRegistry));
    __builtin_memset((void*) m_memory_registry, 0, sizeof(TLMemoryRegistry));
    return TRUE;
}

b8 tl_platform_terminate(void) {
    TLTRACE("tl_platform_terminate(void)")
    if (m_memory_registry->allocated != 0) {
        TLERROR("Memmory leak");
        return FALSE;
    }

    __builtin_free(m_memory_registry);
    return TRUE;
}

#endif