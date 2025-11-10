#ifndef __TELEIOS_MEMORY__
#define __TELEIOS_MEMORY__

#include "teleios/defines.h"

typedef enum {
    TL_MEMORY_BLOCK,
    TL_MEMORY_GRAPHICS,
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

/**
 * @brief Allocator type
 *
 * LINEAR: Arena allocator - fast allocation, no individual deallocation.
 *         Memory is only freed when allocator is destroyed.
 *         Best for subsystems that allocate once and deallocate everything at shutdown.
 *
 * DYNAMIC: Heap allocator - supports individual deallocation with tl_memory_free().
 *          Tracks all allocations and reports memory leaks on destruction.
 *          Best for resources with dynamic lifecycle (threads, dynamic objects, etc).
 */
typedef enum {
    TL_ALLOCATOR_LINEAR,
    TL_ALLOCATOR_DYNAMIC
} TLAllocatorType;

typedef struct TLAllocator TLAllocator;

b8 tl_memory_initialize(void);
b8 tl_memory_terminate(void);

/**
 * @brief Create a memory allocator
 * @param size Page size for LINEAR allocator, or 0 for DYNAMIC allocator
 * @param type Allocator type (LINEAR or DYNAMIC)
 * @return Pointer to allocator, or NULL on failure
 */
TLAllocator* tl_memory_allocator_create(u32 size, TLAllocatorType type);

/**
 * @brief Destroy allocator and report memory leaks (for DYNAMIC allocators)
 * @param allocator Allocator to destroy
 */
void tl_memory_allocator_destroy(TLAllocator* allocator);

/**
 * @brief Allocate memory from allocator
 * @param allocator Allocator to use
 * @param tag Memory tag for tracking
 * @param size Size in bytes
 * @return Pointer to allocated memory, or NULL on failure
 */
void* tl_memory_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size);

/**
 * @brief Free memory allocated from DYNAMIC allocator
 * @param allocator Allocator that owns the memory (must be DYNAMIC type)
 * @param pointer Pointer to free
 * @note Only works with DYNAMIC allocators. For LINEAR allocators, memory is freed on allocator destruction.
 */
void tl_memory_free(TLAllocator* allocator, void* pointer);

void tl_memory_set(void *target, i32 value, u32 size);
void tl_memory_copy(void *target, const void *source, u32 size);

#endif