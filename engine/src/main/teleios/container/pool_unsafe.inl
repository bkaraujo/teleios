#ifndef __TELEIOS_CONTAINER_POOL_UNSAFE__
#define __TELEIOS_CONTAINER_POOL_UNSAFE__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

void* tl_pool_unsafe_acquire(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    for (u16 i = 0; i < pool->capacity; ++i) {
        const u16 index = (pool->next_free + i) % pool->capacity;

        if (!pool->in_use[index]) {
            pool->in_use[index] = true;
            pool->next_free = (index + 1) % pool->capacity;
            pool->mod_count++;

            void* object = pool->memory + (index * pool->object_size);

            TLVERBOSE("Acquired object from pool 0x%p: index=%u, ptr=0x%p", pool, index, object)
            TL_PROFILER_POP_WITH(object)
        }
    }

    // TLWARN("Object pool 0x%p exhausted (capacity=%u)", pool, pool->capacity)
    TL_PROFILER_POP_WITH(NULL)
}

void tl_pool_unsafe_release(TLObjectPool* pool, void* object) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", pool, object)

    const u8* object_ptr = (u8*)object;
    const u64 offset = object_ptr - pool->memory;
    const u16 index = (u16)(offset / pool->object_size);

    if (!pool->in_use[index]) {
        TLWARN("Releasing object 0x%p that was not acquired (index=%u)", object, index)
    }

    pool->in_use[index] = false;
    pool->mod_count++;

    TLVERBOSE("Released object to pool 0x%p: index=%u, ptr=0x%p", pool, index, object)

    TL_PROFILER_POP
}

u16 tl_pool_unsafe_available(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    u16 available = 0;
    for (u16 i = 0; i < pool->capacity; ++i) {
        if (!pool->in_use[i]) {
            available++;
        }
    }

    TL_PROFILER_POP_WITH(available)
}

u16 tl_pool_unsafe_in_use(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    u16 in_use = 0;
    for (u16 i = 0; i < pool->capacity; ++i) {
        if (pool->in_use[i]) {
            in_use++;
        }
    }

    TL_PROFILER_POP_WITH(in_use)
}

u16 tl_pool_unsafe_capacity(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    TL_PROFILER_POP_WITH(pool->capacity)
}

void tl_pool_unsafe_reset(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    pool->next_free = 0;
    tl_memory_set(pool->in_use, 0, sizeof(b8) * pool->capacity);
    pool->mod_count++;

    TLTRACE("Object pool 0x%p reset (%u objects now available)", pool, pool->capacity)

    TL_PROFILER_POP
}

#endif
