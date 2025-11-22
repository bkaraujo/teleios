#ifndef __TELEIOS_CONTAINER_QUEUE_ITERATOR__
#define __TELEIOS_CONTAINER_QUEUE_ITERATOR__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

typedef struct {
    u16 index;                 // Current index in circular buffer
    u16 remaining;             // Number of items left to iterate
} TLQueueIteratorState;

static void tl_queue_iterator_check_modification(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLQueue* queue = (const TLQueue*)iterator->source;

    if (queue->thread_safe) tl_mutex_lock(queue->mutex);
    const u32 current_mod_count = queue->mod_count;
    if (queue->thread_safe) tl_mutex_unlock(queue->mutex);

    if (current_mod_count != iterator->expected_mod_count) {
        TLFATAL("Concurrent modification detected during queue iteration (expected=%u, actual=%u)",
                iterator->expected_mod_count, current_mod_count)
    }

    TL_PROFILER_POP
}

static b8 tl_queue_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLQueueIteratorState* state = (const TLQueueIteratorState*)iterator->state;

    TL_PROFILER_POP_WITH(state->remaining > 0)
}

static void* tl_queue_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLQueueIteratorState* state = (TLQueueIteratorState*)iterator->state;

    if (state->remaining == 0) {
        TLWARN("Iterator exhausted")
        TL_PROFILER_POP_WITH(NULL)
    }

    const TLQueue* queue = (const TLQueue*)iterator->source;

    if (queue->thread_safe) tl_mutex_lock(queue->mutex);
    void* item = queue->items[state->index];
    if (queue->thread_safe) tl_mutex_unlock(queue->mutex);

    state->index = (state->index + 1) % queue->capacity;
    state->remaining--;

    TL_PROFILER_POP_WITH(item)
}

static void tl_queue_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLQueue* queue = (const TLQueue*)iterator->source;
    TLQueueIteratorState* state = (TLQueueIteratorState*)iterator->state;

    if (queue->thread_safe) tl_mutex_lock(queue->mutex);
    state->index = queue->tail;
    state->remaining = queue->count;
    if (queue->thread_safe) tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP
}

static void tl_queue_iterator_resync(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLQueue* queue = (TLQueue*)iterator->source;
    TLQueueIteratorState* state = (TLQueueIteratorState*)iterator->state;

    if (queue->thread_safe) tl_mutex_lock(queue->mutex);

    iterator->expected_mod_count = queue->mod_count;
    iterator->size = queue->count;
    state->index = queue->tail;
    state->remaining = queue->count;

    if (queue->thread_safe) tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP
}

TLIterator* tl_queue_iterator(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLERROR("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (queue->thread_safe) tl_mutex_lock(queue->mutex);

    TLIterator* iterator = tl_memory_alloc(queue->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));
    TLQueueIteratorState* state = tl_memory_alloc(queue->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLQueueIteratorState));

    state->index = queue->tail;
    state->remaining = queue->count;

    iterator->source = queue;
    iterator->expected_mod_count = queue->mod_count;
    iterator->size = queue->count;
    iterator->state = state;
    iterator->allocator = queue->allocator;

    iterator->has_modified = tl_queue_iterator_check_modification;
    iterator->has_next = tl_queue_iterator_has_next;
    iterator->next = tl_queue_iterator_next;
    iterator->rewind = tl_queue_iterator_rewind;
    iterator->resync = tl_queue_iterator_resync;

    if (queue->thread_safe) tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

#endif
