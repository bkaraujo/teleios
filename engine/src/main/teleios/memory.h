#ifndef __TELEIOS_MEMORY__
#define __TELEIOS_MEMORY__

#include "teleios/defines.h"

typedef enum {
    TL_MEMORY_BLOCK,
    TL_MEMORY_SERIALIZER,
    TL_MEMORY_CONTAINER_STACK,
    TL_MEMORY_CONTAINER_LIST,
    TL_MEMORY_CONTAINER_NODE,
    TL_MEMORY_CONTAINER_MAP,
    TL_MEMORY_CONTAINER_MAP_ENTRY,
    TL_MEMORY_CONTAINER_ITERATOR,
    TL_MEMORY_STRING,
    TL_MEMORY_ULID,
    TL_MEMORY_PROFILER,
    TL_MEMORY_SCENE,
    TL_MEMORY_ECS_COMPONENT,
    TL_MEMORY_THREAD,
    TL_MEMORY_MAXIMUM
} TLMemoryTag;

typedef struct TLAllocator TLAllocator;

b8 tl_memory_initialize(void);
b8 tl_memory_terminate(void);

TLAllocator* tl_memory_allocator_create(u32 size);
void tl_memory_allocator_destroy(TLAllocator* allocator);

void* tl_memory_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size);
void tl_memory_set(void *target, i32 value, u32 size);
void tl_memory_copy(void *target, const void *source, u32 size);

#endif