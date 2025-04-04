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

static const char* tl_memory_name(const TLMemoryTag tag) {
    TL_STACK_PUSHA("%d", tag)
    switch (tag) {
        case TL_MEMORY_BLOCK                : TL_STACK_POPV("TL_MEMORY_BLOCK")
        case TL_MEMORY_SERIALIZER           : TL_STACK_POPV("TL_MEMORY_SERIALIZER")
        case TL_MEMORY_CONTAINER_LIST       : TL_STACK_POPV("TL_MEMORY_CONTAINER_LIST")
        case TL_MEMORY_CONTAINER_STACK      : TL_STACK_POPV("TL_MEMORY_CONTAINER_STACK")
        case TL_MEMORY_CONTAINER_NODE       : TL_STACK_POPV("TL_MEMORY_CONTAINER_NODE")
        case TL_MEMORY_CONTAINER_MAP        : TL_STACK_POPV("TL_MEMORY_CONTAINER_MAP")
        case TL_MEMORY_CONTAINER_MAP_ENTRY  : TL_STACK_POPV("TL_MEMORY_CONTAINER_MAP_ENTRY")
        case TL_MEMORY_CONTAINER_ITERATOR   : TL_STACK_POPV("TL_MEMORY_CONTAINER_ITERATOR")
        case TL_MEMORY_STRING               : TL_STACK_POPV("TL_MEMORY_STRING")
        case TL_MEMORY_PROFILER             : TL_STACK_POPV("TL_MEMORY_PROFILER")
        case TL_MEMORY_SCENE                : TL_STACK_POPV("TL_MEMORY_SCENE")
        case TL_MEMORY_ECS_COMPONENT        : TL_STACK_POPV("TL_MEMORY_ECS_COMPONENT")
        case TL_MEMORY_ULID                 : TL_STACK_POPV("TL_MEMORY_ULID")
        case TL_MEMORY_THREAD               : TL_STACK_POPV("TL_MEMORY_THREAD")
        case TL_MEMORY_MAXIMUM              : TL_STACK_POPV("TL_MEMORY_MAXIMUM")
    }

    TL_STACK_POPV("TL_MEMORY_???")
}

TLMemoryArena* tl_memory_arena_create(const u64 size) {
    TL_STACK_PUSHA("%d", size)
    // ----------------------------------------------------------
    // Create the memory arena
    // ----------------------------------------------------------
    TLMemoryArena *arena = tl_platform_memory_alloc(sizeof(TLMemoryArena));
    if (arena == NULL) TLFATAL("Failed to allocate TLMemoryArena");
    tl_platform_memory_set(arena, 0, sizeof(TLMemoryArena));
    arena->page_size = size;
    // ----------------------------------------------------------
    // Keep track of the created arena
    // ----------------------------------------------------------
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] == NULL) {
            arenas[i] = arena;
            TLTRACE("TLMemoryArena 0x%p created with page size of %d bytes", arena, arena->page_size)
            TL_STACK_POPV(arena)
        }
    }

    TLFATAL("Failed to allocate TLMemoryArena");
}

TL_INLINE static u8 tl_memory_arena_get_index(const TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)

    if (arena == NULL) {
        TLWARN("TLMemoryArena is NULL")
        TL_STACK_POPV(false)
    }

    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (arenas[i] == NULL) continue;
        if (arenas[i] == arena) {
            TL_STACK_POPV(i)
        }
    }

    TLFATAL("TLMemoryArena 0x%p not found", arena)
}

TL_INLINE static void tl_memory_arena_do_destroy(const u8 index) {
    TL_STACK_PUSHA("%d", index)
    TLMemoryArena *arena = arenas[index];
    for (u32 i = 0 ; i < TL_ARR_LENGTH(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload != NULL) {
            TLVERBOSE("TLMemoryArena 0x%p releasing page %d", arena, i)
            tl_platform_memory_free(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (arenas[index]->tagged_size[i] != 0) {
            TLVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    tl_platform_memory_free(arena);
    arenas[index] = NULL;

    TL_STACK_POP
}

void tl_memory_arena_reset(TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)
    for (u32 i = 0 ; i < TL_ARR_LENGTH(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload == NULL) break;

        arena->page[i].index = 0;
        tl_memory_set(arena->page[i].payload, 0, arena->page_size);
    }
    TL_STACK_POP
}

void tl_memory_arena_destroy(TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)
    const u8 index = tl_memory_arena_get_index(arena);
    tl_memory_arena_do_destroy(index);
    TL_STACK_POP
}

void *tl_memory_alloc(TLMemoryArena *arena, const u64 size, const TLMemoryTag tag) {
    TL_STACK_PUSHA("0x%p, %llu, %d", arena, size, tag)
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (size == 0) {
        TLFATAL("TLMemoryArena 0x%p allocation size must be greater then 0", arena)
        TL_STACK_POPV(NULL)
    }

    if (size > arena->page_size) {
        TLFATAL("TLMemoryArena with page size of %d bytes. It cannot allocate %d bytes", arena, arena->page_size, size)
        TL_STACK_POPV(NULL)
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {

        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            TLTRACE("TLMemoryArena 0x%p initializing page %d", arena, i)
            arena->page[i].payload = tl_platform_memory_alloc(arena->page_size);
            tl_platform_memory_set(arena->page[i].payload, 0, arena->page_size);

            found = i;
            break;
        }

        // check if the page support the desired size
        if (arena->page[i].index + size <= arena->page_size) {
            found = i;
            break;
        }
    }

    if (found == U8_MAX) {
        TLWARN("TLMemoryArena 0x%p no suitable TLMemoryPage", arena)
        TL_STACK_POPV(NULL)
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
    TLVERBOSE("TLMemoryArena 0x%p page %d [remaning %llu] :: allocating %llu ", arena, found, arena->page_size - arena->page[found].index, size)
    arena->page[found].index += size;
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    TL_STACK_POPV(address)
}

void tl_memory_set(void *block, const i32 value, const u64 size) {
    TL_STACK_PUSHA("0x%p, %d, %llu", block, value, size)
    tl_platform_memory_set(block, value, size);
    TL_STACK_POP
}

void tl_memory_copy(void *target, void *source, const u64 size) {
    TL_STACK_PUSHA("0x%p, 0x%p, %llu", target, source, size)
    tl_platform_memory_copy(target, source, size);
    TL_STACK_POP
}