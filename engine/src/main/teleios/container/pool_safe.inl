#ifndef __TELEIOS_CONTAINER_POOL_SAFE__
#define __TELEIOS_CONTAINER_POOL_SAFE__

#include "teleios/teleios.h"
#include "teleios/container/pool_unsafe.inl"

void* tl_pool_safe_acquire(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    tl_mutex_lock(pool->mutex);
    void* result = tl_pool_unsafe_acquire(pool);
    tl_mutex_unlock(pool->mutex);
    TL_PROFILER_POP_WITH(result)
}

void tl_pool_safe_release(TLObjectPool* pool, void* object) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", pool, object)
    tl_mutex_lock(pool->mutex);
    tl_pool_unsafe_release(pool, object);
    tl_mutex_unlock(pool->mutex);
    TL_PROFILER_POP
}

u16 tl_pool_safe_available(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    tl_mutex_lock(pool->mutex);
    const u16 result = tl_pool_unsafe_available(pool);
    tl_mutex_unlock(pool->mutex);
    TL_PROFILER_POP_WITH(result)
}

u16 tl_pool_safe_in_use(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    tl_mutex_lock(pool->mutex);
    const u16 result = tl_pool_unsafe_in_use(pool);
    tl_mutex_unlock(pool->mutex);
    TL_PROFILER_POP_WITH(result)
}

u16 tl_pool_safe_capacity(const TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    tl_mutex_lock(pool->mutex);
    const u16 result = tl_pool_unsafe_capacity(pool);
    tl_mutex_unlock(pool->mutex);
    TL_PROFILER_POP_WITH(result)
}

void tl_pool_safe_reset(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)
    tl_mutex_lock(pool->mutex);
    tl_pool_unsafe_reset(pool);
    tl_mutex_unlock(pool->mutex);
    TL_PROFILER_POP
}

#endif
