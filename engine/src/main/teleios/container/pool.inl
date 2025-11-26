#ifndef __TELEIOS_CONTAINER_POOL__
#define __TELEIOS_CONTAINER_POOL__

#include "teleios/memory/types.inl"
#include "teleios/container/types.inl"
#include "teleios/teleios.h"
#include "teleios/container/pool_safe.inl"
#include "teleios/container/pool_unsafe.inl"

// ---------------------------------
// TLObjectPool Implementation
// ---------------------------------

TLObjectPool* tl_pool_create(TLAllocator* allocator, const u32 object_size, const u16 capacity, const b8 thread_safe) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, %u, %d", allocator, object_size, capacity, thread_safe)

    if (allocator == NULL) {
        TLERROR("Attempted to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (object_size == 0) {
        TLERROR("Attempted to use object_size 0")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (capacity == 0) {
        TLERROR("Attempted to use capacity 0")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLObjectPool* pool = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_POOL, sizeof(TLObjectPool));
    pool->memory = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_POOL, object_size * capacity);
    pool->in_use = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_POOL, sizeof(b8) * capacity);
    pool->object_size = object_size;
    pool->capacity = capacity;
    pool->next_free = 0;
    pool->mod_count = 0;
    pool->allocator = allocator;
    pool->thread_safe = thread_safe;
    pool->mutex = NULL;
    pool->not_empty = NULL;

    // Zero-initialize memory and bitmap
    tl_memory_set(pool->memory, 0, object_size * capacity);
    tl_memory_set(pool->in_use, 0, sizeof(b8) * capacity);

    if (thread_safe) {
        pool->mutex = tl_mutex_create(allocator);
        if (!pool->mutex) {
            TLERROR("Failed to create mutex for pool")
            tl_memory_free(allocator, pool->in_use);
            tl_memory_free(allocator, pool->memory);
            tl_memory_free(allocator, pool);
            TL_PROFILER_POP_WITH(NULL)
        }

        pool->not_empty = tl_condition_create(allocator);
        if (!pool->not_empty) {
            TLERROR("Failed to create condition for pool")
            tl_mutex_destroy(pool->mutex);
            tl_memory_free(allocator, pool->in_use);
            tl_memory_free(allocator, pool->memory);
            tl_memory_free(allocator, pool);
            TL_PROFILER_POP_WITH(NULL)
        }
    }

    TLTRACE("Object pool created: thread_safe=%d, capacity=%u, object_size=%u, total_mem=%u bytes",
        thread_safe, capacity, object_size, object_size * capacity)

    TL_PROFILER_POP_WITH(pool)
}

void tl_pool_destroy(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to destroy a NULL TLObjectPool")
        TL_PROFILER_POP
    }

    TLTRACE("Destroying object pool 0x%p", pool)

    if (pool->not_empty) tl_condition_destroy(pool->not_empty);
    if (pool->mutex) tl_mutex_destroy(pool->mutex);
    tl_memory_free(pool->allocator, pool->in_use);
    tl_memory_free(pool->allocator, pool->memory);
    tl_memory_free(pool->allocator, pool);

    TL_PROFILER_POP
}

// ---------------------------------
// TLObjectPool Dispatchers
// ---------------------------------

void* tl_pool_acquire(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (pool->thread_safe) TL_PROFILER_POP_WITH(tl_pool_safe_acquire(pool));
    TL_PROFILER_POP_WITH(tl_pool_unsafe_acquire(pool));
}

void* tl_pool_acquire_wait(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (!pool->thread_safe) {
        TLFATAL("tl_pool_acquire_wait requires thread_safe=true")
    }

    TL_PROFILER_POP_WITH(tl_pool_safe_acquire_wait(pool));
}

void tl_pool_release(TLObjectPool* pool, void* object) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", pool, object)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP
    }

    if (object == NULL) {
        TLERROR("Attempted to release a NULL object")
        TL_PROFILER_POP
    }

    // Validation before dispatch
    const u8* object_ptr = (u8*)object;
    const u64 offset = object_ptr - pool->memory;

    if (offset % pool->object_size != 0) {
        TLFATAL("Object 0x%p is not aligned to object_size %u (offset=%llu)", object, pool->object_size, offset)
    }

    const u16 index = (u16)(offset / pool->object_size);
    if (index >= pool->capacity) {
        TLFATAL("Object 0x%p is outside pool bounds (index=%u, capacity=%u)", object, index, pool->capacity)
    }

    if (pool->thread_safe) {
        tl_pool_safe_release(pool, object);
        TL_PROFILER_POP
    }
    tl_pool_unsafe_release(pool, object);
    TL_PROFILER_POP
}

u16 tl_pool_available(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP_WITH(0)
    }

    if (pool->thread_safe) TL_PROFILER_POP_WITH(tl_pool_safe_available(pool));
    TL_PROFILER_POP_WITH(tl_pool_unsafe_available(pool));
}

u16 tl_pool_in_use(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP_WITH(0)
    }

    if (pool->thread_safe) TL_PROFILER_POP_WITH(tl_pool_safe_in_use(pool));
    TL_PROFILER_POP_WITH(tl_pool_unsafe_in_use(pool));
}

u16 tl_pool_capacity(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP_WITH(0)
    }

    if (pool->thread_safe) TL_PROFILER_POP_WITH(tl_pool_safe_capacity(pool));
    TL_PROFILER_POP_WITH(tl_pool_unsafe_capacity(pool));
}

void tl_pool_reset(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP
    }

    if (pool->thread_safe) {
        tl_pool_safe_reset(pool);
        TL_PROFILER_POP
    }
    tl_pool_unsafe_reset(pool);
    TL_PROFILER_POP
}

#endif
