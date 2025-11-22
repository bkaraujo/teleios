#ifndef __TELEIOS_CONTAINER_POOL_ITERATOR__
#define __TELEIOS_CONTAINER_POOL_ITERATOR__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

typedef struct {
    u16 index;
} TLPoolIteratorState;

static void tl_pool_iterator_check_modification(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLObjectPool* pool = (const TLObjectPool*)iterator->source;

    if (pool->thread_safe) tl_mutex_lock(pool->mutex);
    const u32 current_mod_count = pool->mod_count;
    if (pool->thread_safe) tl_mutex_unlock(pool->mutex);

    if (current_mod_count != iterator->expected_mod_count) {
        TLFATAL("Concurrent modification detected during pool iteration (expected=%u, actual=%u)",
                iterator->expected_mod_count, current_mod_count)
    }

    TL_PROFILER_POP
}

static b8 tl_pool_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLObjectPool* pool = (const TLObjectPool*)iterator->source;
    const TLPoolIteratorState* state = (const TLPoolIteratorState*)iterator->state;

    for (u16 i = state->index; i < pool->capacity; ++i) {
        if (pool->in_use[i]) {
            TL_PROFILER_POP_WITH(true)
        }
    }

    TL_PROFILER_POP_WITH(false)
}

static void* tl_pool_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLObjectPool* pool = (TLObjectPool*)iterator->source;
    TLPoolIteratorState* state = (TLPoolIteratorState*)iterator->state;

    while (state->index < pool->capacity) {
        if (pool->in_use[state->index]) {
            void* object = pool->memory + (state->index * pool->object_size);
            state->index++;
            TL_PROFILER_POP_WITH(object)
        }
        state->index++;
    }

    TLWARN("Iterator exhausted")
    TL_PROFILER_POP_WITH(NULL)
}

static void tl_pool_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLObjectPool* pool = (const TLObjectPool*)iterator->source;
    TLPoolIteratorState* state = (TLPoolIteratorState*)iterator->state;

    if (pool->thread_safe) tl_mutex_lock(pool->mutex);
    state->index = 0;
    if (pool->thread_safe) tl_mutex_unlock(pool->mutex);

    TL_PROFILER_POP
}

static void tl_pool_iterator_resync(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLObjectPool* pool = (TLObjectPool*)iterator->source;
    TLPoolIteratorState* state = (TLPoolIteratorState*)iterator->state;

    if (pool->thread_safe) tl_mutex_lock(pool->mutex);

    iterator->expected_mod_count = pool->mod_count;

    u32 count = 0;
    for (u16 i = 0; i < pool->capacity; ++i) {
        if (pool->in_use[i]) {
            count++;
        }
    }
    iterator->size = count;

    state->index = 0;

    if (pool->thread_safe) tl_mutex_unlock(pool->mutex);

    TL_PROFILER_POP
}

TLIterator* tl_pool_iterator(TLObjectPool* pool) {
    TL_PROFILER_PUSH_WITH("0x%p", pool)

    if (pool == NULL) {
        TLERROR("Attempted to use a NULL TLObjectPool")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (pool->thread_safe) tl_mutex_lock(pool->mutex);

    TLIterator* iterator = tl_memory_alloc(pool->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));
    TLPoolIteratorState* state = tl_memory_alloc(pool->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLPoolIteratorState));

    state->index = 0;

    u32 count = 0;
    for (u16 i = 0; i < pool->capacity; ++i) {
        if (pool->in_use[i]) {
            count++;
        }
    }

    iterator->source = pool;
    iterator->expected_mod_count = pool->mod_count;
    iterator->size = count;
    iterator->state = state;
    iterator->allocator = pool->allocator;

    iterator->has_modified = tl_pool_iterator_check_modification;
    iterator->has_next = tl_pool_iterator_has_next;
    iterator->next = tl_pool_iterator_next;
    iterator->rewind = tl_pool_iterator_rewind;
    iterator->resync = tl_pool_iterator_resync;

    if (pool->thread_safe) tl_mutex_unlock(pool->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

#endif
