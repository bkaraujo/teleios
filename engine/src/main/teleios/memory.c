#include "teleios/core.h"
#include "teleios/runtime.h"

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

static TLMemoryArena *arenas[U8_MAX];

b8 tl_memory_initialize(void) {
    BKS_TRACE_PUSH
    bks_memory_set(arenas, 0, sizeof(arenas));
    BKS_TRACE_POPV(true)
}

b8 tl_memory_terminate(void) {
    BKS_TRACE_PUSH
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] != NULL) {
            BKSERROR("TLMemoryArena 0x%p is live", arenas[i]);
        }
    }
    BKS_TRACE_POPV(true)
}

static const char* tl_memory_name(const TLMemoryTag tag) {
    BKS_TRACE_PUSHA("%d", tag)
    switch (tag) {
        case TL_MEMORY_BLOCK                : BKS_TRACE_POPV("TL_MEMORY_BLOCK")
        case TL_MEMORY_SERIALIZER           : BKS_TRACE_POPV("TL_MEMORY_SERIALIZER")
        case TL_MEMORY_CONTAINER_LIST       : BKS_TRACE_POPV("TL_MEMORY_CONTAINER_LIST")
        case TL_MEMORY_CONTAINER_STACK      : BKS_TRACE_POPV("TL_MEMORY_CONTAINER_STACK")
        case TL_MEMORY_CONTAINER_NODE       : BKS_TRACE_POPV("TL_MEMORY_CONTAINER_NODE")
        case TL_MEMORY_CONTAINER_MAP        : BKS_TRACE_POPV("TL_MEMORY_CONTAINER_MAP")
        case TL_MEMORY_CONTAINER_MAP_ENTRY  : BKS_TRACE_POPV("TL_MEMORY_CONTAINER_MAP_ENTRY")
        case TL_MEMORY_CONTAINER_ITERATOR   : BKS_TRACE_POPV("TL_MEMORY_CONTAINER_ITERATOR")
        case TL_MEMORY_STRING               : BKS_TRACE_POPV("TL_MEMORY_STRING")
        case TL_MEMORY_PROFILER             : BKS_TRACE_POPV("TL_MEMORY_PROFILER")
        case TL_MEMORY_SCENE                : BKS_TRACE_POPV("TL_MEMORY_SCENE")
        case TL_MEMORY_ECS_COMPONENT        : BKS_TRACE_POPV("TL_MEMORY_ECS_COMPONENT")
        case TL_MEMORY_ULID                 : BKS_TRACE_POPV("TL_MEMORY_ULID")
        case TL_MEMORY_THREAD               : BKS_TRACE_POPV("TL_MEMORY_THREAD")
        case TL_MEMORY_MAXIMUM              : BKS_TRACE_POPV("TL_MEMORY_MAXIMUM")
    }

    BKS_TRACE_POPV("TL_MEMORY_???")
}

TLMemoryArena* tl_memory_arena_create(const u64 size) {
    BKS_TRACE_PUSHA("%d", size)
    // ----------------------------------------------------------
    // Create the memory arena
    // ----------------------------------------------------------
    TLMemoryArena *arena = bks_memory_alloc(sizeof(TLMemoryArena));
    if (arena == NULL) BKSFATAL("Failed to allocate TLMemoryArena");
    bks_memory_set(arena, 0, sizeof(TLMemoryArena));
    arena->page_size = size;
    // ----------------------------------------------------------
    // Keep track of the created arena
    // ----------------------------------------------------------
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] == NULL) {
            arenas[i] = arena;
            BKSDEBUG("TLMemoryArena 0x%p created with page length of %d bytes", arena, arena->page_size)
            BKS_TRACE_POPV(arena)
        }
    }

    BKSFATAL("Failed to allocate TLMemoryArena");
}

void *tl_memory_alloc(TLMemoryArena *arena, const u64 size, const TLMemoryTag tag) {
    BKS_TRACE_PUSHA("0x%p, %llu, %d", arena, size, tag)
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired length
    // -------------------------------------------------
    if (size == 0) {
        BKSFATAL("TLMemoryArena 0x%p allocation length must be greater then 0", arena)
        BKS_TRACE_POPV(NULL)
    }

    if (size > arena->page_size) {
        BKSFATAL("TLMemoryArena with page length of %d bytes. It cannot allocate %d bytes", arena, arena->page_size, size)
        BKS_TRACE_POPV(NULL)
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {

        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            arena->page[i].payload = bks_memory_alloc(arena->page_size);
            BKSTRACE("TLMemoryArena 0x%p initializing page %d at 0x%p", arena, i, arena->page[i].payload)
            bks_memory_set(arena->page[i].payload, 0, arena->page_size);

            found = i;
            break;
        }

        // check if the page support the desired length
        if (arena->page[i].index + size <= arena->page_size) {
            found = i;
            break;
        }
    }

    if (found == U8_MAX) {
        BKSWARN("TLMemoryArena 0x%p no suitable TLMemoryPage", arena)
        BKS_TRACE_POPV(NULL)
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
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    BKS_TRACE_POPV(address)
}

BKS_INLINE static u8 tl_memory_arena_get_index(const TLMemoryArena *arena) {
    BKS_TRACE_PUSHA("0x%p", arena)

    if (arena == NULL) {
        BKSWARN("TLMemoryArena is NULL")
        BKS_TRACE_POPV(false)
    }

    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] == NULL) continue;
        if (arenas[i] == arena) {
            BKS_TRACE_POPV(i)
        }
    }

    BKSFATAL("TLMemoryArena 0x%p not found", arena)
}

BKS_INLINE static void tl_memory_arena_do_destroy(const u8 index) {
    BKS_TRACE_PUSHA("%d", index)
    TLMemoryArena *arena = arenas[index];
    for (u32 i = 0 ; i < TL_ARR_LENGTH(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload != NULL) {
            BKSDEBUG("TLMemoryArena 0x%p releasing page %d at 0x%p", arena, i, arena->page[i].payload)
            bks_memory_free(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (arenas[index]->tagged_size[i] != 0) {
            BKSVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    bks_memory_free(arena);
    arenas[index] = NULL;

    BKS_TRACE_POP
}

void tl_memory_arena_reset(TLMemoryArena *arena) {
    BKS_TRACE_PUSHA("0x%p", arena)
    for (u32 i = 0 ; i < TL_ARR_LENGTH(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload == NULL) break;

        arena->page[i].index = 0;
        tl_memory_set(arena->page[i].payload, 0, arena->page_size);
    }
    BKS_TRACE_POP
}

void tl_memory_arena_destroy(TLMemoryArena *arena) {
    BKS_TRACE_PUSHA("0x%p", arena)
    const u8 index = tl_memory_arena_get_index(arena);
    tl_memory_arena_do_destroy(index);
    BKS_TRACE_POP
}

void tl_memory_set(void *block, const i32 value, const u64 size) {
    BKS_TRACE_PUSHA("0x%p, %d, %llu", block, value, size)
    bks_memory_set(block, value, size);
    BKS_TRACE_POP
}

void tl_memory_copy(void *target, void *source, const u64 size) {
    BKS_TRACE_PUSHA("0x%p, 0x%p, %llu", target, source, size)
    bks_memory_copy(target, source, size);
    BKS_TRACE_POP
}