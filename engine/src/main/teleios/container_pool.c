#include "memory_types.inl"
#include "teleios/teleios.h"
#include "teleios/container_types.inl"

// ---------------------------------
// Object Pool Lifecycle
// ---------------------------------

TLObjectPool* tl_pool_create(TLAllocator* allocator, const u32 object_size, const u16 capacity) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, %u", allocator, object_size, capacity)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (object_size == 0) TLFATAL("object_size is 0")
    if (capacity == 0) TLFATAL("capacity is 0")

    // Allocate pool structure
    TLObjectPool* pool = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_POOL, sizeof(TLObjectPool));

    // Allocate contiguous memory block for all objects
    pool->memory = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_POOL, object_size * capacity);

    // Allocate in_use bitmap
    pool->in_use = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_POOL, sizeof(b8) * capacity);

    // Initialize pool metadata
    pool->object_size = object_size;
    pool->capacity = capacity;
    pool->next_free = 0;
    pool->allocator = allocator;

    // Create mutex for thread-safety
    pool->mutex = tl_mutex_create(allocator);
    if (!pool->mutex) {
        TLFATAL("Failed to create mutex for object pool")
    }

    // Zero-initialize memory and bitmap
    tl_memory_set(pool->memory, 0, object_size * capacity);
    tl_memory_set(pool->in_use, 0, sizeof(b8) * capacity);

    TLDEBUG("Object pool created: 0x%p (capacity=%u, object_size=%u, total_mem=%u bytes, thread-safe)",
        pool, capacity, object_size, object_size * capacity)

    TL_PROFILER_POP_WITH(pool)
}

void tl_pool_destroy(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLWARN("Attempted to destroy NULL pool")
        TL_PROFILER_POP
    }

    TLDEBUG("Destroying object pool 0x%p", pool)

    // Destroy mutex
    if (pool->mutex) {
        tl_mutex_destroy(pool->mutex);
    }

    tl_memory_free(pool->allocator, pool->memory);
    tl_memory_free(pool->allocator, pool->in_use);
    tl_memory_free(pool->allocator, pool);

    TL_PROFILER_POP
}

// ---------------------------------
// Object Pool Operations
// ---------------------------------

void* tl_pool_acquire(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) TLFATAL("pool is NULL")

    tl_mutex_lock(pool->mutex);

    // Search for a free object starting from next_free hint
    for (u16 i = 0; i < pool->capacity; ++i) {
        const u16 index = (pool->next_free + i) % pool->capacity;

        if (!pool->in_use[index]) {
            // Found free object
            pool->in_use[index] = true;
            pool->next_free = (index + 1) % pool->capacity;

            // Calculate pointer to object
            void* object = pool->memory + (index * pool->object_size);

            tl_mutex_unlock(pool->mutex);
            TLTRACE("Acquired object from pool 0x%p: index=%u, ptr=0x%p", pool, index, object)
            TL_PROFILER_POP_WITH(object)
        }
    }

    // Pool exhausted
    tl_mutex_unlock(pool->mutex);
    TLWARN("Object pool 0x%p exhausted (capacity=%u)", pool, pool->capacity)
    TL_PROFILER_POP_WITH(NULL)
}

void tl_pool_release(TLObjectPool* pool, void* object) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", pool, object)

    if (pool == NULL) TLFATAL("pool is NULL")
    if (object == NULL) TLFATAL("object is NULL")

    // Calculate index from object pointer
    const u8* object_ptr = (u8*)object;
    const u64 offset = object_ptr - pool->memory;

    if (offset % pool->object_size != 0) {
        TLFATAL("Object 0x%p is not aligned to object_size %u (offset=%llu)",
            object, pool->object_size, offset)
    }

    const u16 index = (u16)(offset / pool->object_size);

    if (index >= pool->capacity) {
        TLFATAL("Object 0x%p is outside pool bounds (index=%u, capacity=%u)",
            object, index, pool->capacity)
    }

    tl_mutex_lock(pool->mutex);

    if (!pool->in_use[index]) {
        TLWARN("Releasing object 0x%p that was not acquired (index=%u)", object, index)
    }

    // Mark as free
    pool->in_use[index] = false;
    // Don't update next_free here - let it continue round-robin from where it left off
    // This prevents LIFO behavior that causes the same object to be reused immediately

    tl_mutex_unlock(pool->mutex);

    TLTRACE("Released object to pool 0x%p: index=%u, ptr=0x%p", pool, index, object)

    TL_PROFILER_POP
}

// ---------------------------------
// Object Pool Queries
// ---------------------------------

u16 tl_pool_available(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    if (pool == NULL) TL_PROFILER_POP_WITH(0)

    tl_mutex_lock(pool->mutex);

    u16 available = 0;
    for (u16 i = 0; i < pool->capacity; ++i) {
        if (!pool->in_use[i]) {
            available++;
        }
    }

    tl_mutex_unlock(pool->mutex);

    TL_PROFILER_POP_WITH(available);
}

u16 tl_pool_in_use(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    if (pool == NULL) TL_PROFILER_POP_WITH(0)

    tl_mutex_lock(pool->mutex);

    u16 in_use = 0;
    for (u16 i = 0; i < pool->capacity; ++i) {
        if (pool->in_use[i]) {
            in_use++;
        }
    }

    tl_mutex_unlock(pool->mutex);

    TL_PROFILER_POP_WITH(in_use)
}

u16 tl_pool_capacity(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    if (pool == NULL) TL_PROFILER_POP_WITH(0)
    TL_PROFILER_POP_WITH(pool->capacity);
}

void tl_pool_reset(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLWARN("Attempted to reset NULL pool")
        TL_PROFILER_POP
    }

    tl_mutex_lock(pool->mutex);

    // Mark all objects as free
    tl_memory_set(pool->in_use, 0, sizeof(b8) * pool->capacity);
    pool->next_free = 0;

    tl_mutex_unlock(pool->mutex);

    TLDEBUG("Object pool 0x%p reset (%u objects now available)", pool, pool->capacity)

    TL_PROFILER_POP
}
