#ifndef __TELEIOS_RUNTIME_MEMORY__
#define __TELEIOS_RUNTIME_MEMORY__

typedef enum {
    TL_MEMORY_BLOCK,
    TL_MEMORY_SERIALIZER,
    TL_MEMORY_CONTAINER_STACK,
    TL_MEMORY_CONTAINER_LIST,
    TL_MEMORY_CONTAINER_NODE,
    TL_MEMORY_CONTAINER_ITERATOR,
    TL_MEMORY_STRING,
    TL_MEMORY_ULID,
    TL_MEMORY_PROFILER,
    TL_MEMORY_SCENE,
    TL_MEMORY_MAXIMUM
} TLMemoryTag;

#include "teleios/defines.h"

TLMemoryArena* tl_memory_arena_create(u64 size);
void tl_memory_arena_destroy(TLMemoryArena *arena);
void tl_memory_arena_reset(TLMemoryArena *arena);

void* tl_memory_alloc(TLMemoryArena *arena, u64 size, TLMemoryTag tag);
void tl_memory_set(void *block, i32 value, u64 size);
void tl_memory_copy(void *target, void *source, u64 size);

#endif // __TELEIOS_RUNTIME_MEMORY__