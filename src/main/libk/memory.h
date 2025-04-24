#ifndef __LIBK_MEMORY__
#define __LIBK_MEMORY__

#include "libk/defines.h"

typedef enum {
    K_MEMORY_STRING             = 30000,
    K_MEMORY_COLLECTION,
    K_MEMORY_COLLECTION_KEY,
    K_MEMORY_COLLECTION_VALUE,
    K_MEMORY_COLLECTION_ITERATOR,
    K_MEMORY_CONTAINER_STACK,
    K_MEMORY_CONTAINER_LIST,
    K_MEMORY_CONTAINER_NODE,
    K_MEMORY_CONTAINER_MAP,
    K_MEMORY_CONTAINER_MAP_ENTRY,
    K_MEMORY_CONTAINER_ITERATOR,
    K_MEMORY_MAXIMUM
} KMemoryTag;

typedef enum {
    K_MEMORY_ALLOCATOR_LINEAR,
    K_MEMORY_ALLOCATOR_DYNAMIC
  } KAllocatorType;

KAllocator* k_memory_allocator_create(KAllocatorType type, u64 size);
void* k_memory_allocator_alloc(KAllocator* allocator, u64 size, u16 tag);
void* k_memory_allocator_free(KAllocator* allocator, void *pointer);
void k_memory_allocator_reset(KAllocator* allocator);
void k_memory_allocator_destroy(KAllocator* allocator);

void* k_memory_alloc(u64 size);
void k_memory_free(void *block);
void k_memory_set(void *target, i32 value, u64 size);
void k_memory_copy(void *target, const void *source, u64 size);

#define k_memory_alloc_stack(block,size)                                    \
    void* block = alloca(size);                                             \
    if (block == NULL) KFATAL("Failed to stack allocate %llu bytes", size)  \
    k_memory_set(block, 0, size);                                           \

#define k_memory_free_stack(block) { _freea(block); }

#endif
