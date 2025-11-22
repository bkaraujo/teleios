#ifndef __TELEIOS_CONTAINER_QUEUE__
#define __TELEIOS_CONTAINER_QUEUE__

#include "teleios/memory/types.inl"
#include "teleios/container/types.inl"
#include "teleios/teleios.h"
#include "teleios/container/queue_safe.inl"
#include "teleios/container/queue_unsafe.inl"

// ---------------------------------
// TLQueue Implementation
// ---------------------------------

TLQueue* tl_queue_create(TLAllocator* allocator, const u16 capacity, const b8 thread_safe) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, %d", allocator, capacity, thread_safe)

    if (allocator == NULL) {
        TLERROR("Attempted to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (capacity == 0) {
        TLERROR("Attempted to use a capacity of 0")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLQueue* queue = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_QUEUE, sizeof(TLQueue));
    if (queue == NULL) {
        TLERROR("Failed to allocate TLQueue structure")
        TL_PROFILER_POP_WITH(NULL)
    }

    queue->items = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_QUEUE, sizeof(void*) * capacity);
    if (queue->items == NULL) {
        TLERROR("Failed to allocate queue items memory")
        tl_memory_free(allocator, queue);
        TL_PROFILER_POP_WITH(NULL)
    }

    queue->capacity = capacity;
    queue->allocator = allocator;
    queue->thread_safe = thread_safe;

    if (thread_safe) {
        queue->mutex = tl_mutex_create(allocator);
        queue->not_empty = tl_condition_create(allocator);
        queue->not_full = tl_condition_create(allocator);

        if (!queue->mutex || !queue->not_empty || !queue->not_full) {
            TLERROR("Failed to create synchronization primitives for queue")
            if (queue->mutex) tl_mutex_destroy(queue->mutex);
            if (queue->not_empty) tl_condition_destroy(queue->not_empty);
            if (queue->not_full) tl_condition_destroy(queue->not_full);
            tl_memory_free(allocator, queue->items);
            tl_memory_free(allocator, queue);
            TL_PROFILER_POP_WITH(NULL)
        }
    }

    TLTRACE("Queue created: capacity=%u, thread_safe=%d", capacity, thread_safe);
    TL_PROFILER_POP_WITH(queue)
}

void tl_queue_destroy(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TL_PROFILER_POP
    }

    TLTRACE("Destroying queue: count=%u, capacity=%u", queue->count, queue->capacity);

    if (queue->mutex) tl_mutex_destroy(queue->mutex);
    if (queue->not_empty) tl_condition_destroy(queue->not_empty);
    if (queue->not_full) tl_condition_destroy(queue->not_full);
    tl_memory_free(queue->allocator, queue->items);
    tl_memory_free(queue->allocator, queue);

    TL_PROFILER_POP
}

// ---------------------------------
// TLQueue Dispatchers
// ---------------------------------

void tl_queue_offer(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP
    }

    if (queue->count >= queue->capacity) {
        TLWARN("Queue is full, cannot offer payload")
        TL_PROFILER_POP
    }

    if (queue->thread_safe) {
        tl_queue_safe_offer(queue, payload);
        TL_PROFILER_POP
    }
    tl_queue_unsafe_offer(queue, payload);
    TL_PROFILER_POP
}

void tl_queue_push(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP
    }

    if (queue->thread_safe) {
        tl_queue_safe_push(queue, payload);
        TL_PROFILER_POP
    }

    // Non thread-safe: just push without blocking
    if (queue->count >= queue->capacity) {
        TLWARN("Queue is full, cannot push payload")
        TL_PROFILER_POP
    }
    tl_queue_unsafe_push(queue, payload);
    TL_PROFILER_POP
}

void* tl_queue_pop(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (queue->count == 0) {
        TL_PROFILER_POP_WITH(NULL)
    }

    if (queue->thread_safe) TL_PROFILER_POP_WITH(tl_queue_safe_pop(queue));
    TL_PROFILER_POP_WITH(tl_queue_unsafe_pop(queue));
}

void* tl_queue_peek(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (queue->count == 0) {
        TL_PROFILER_POP_WITH(NULL)
    }

    if (queue->thread_safe) TL_PROFILER_POP_WITH(tl_queue_safe_peek(queue));
    TL_PROFILER_POP_WITH(tl_queue_unsafe_peek(queue));
}

u16 tl_queue_size(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(0)
    }

    if (queue->thread_safe) TL_PROFILER_POP_WITH(tl_queue_safe_size(queue));
    TL_PROFILER_POP_WITH(tl_queue_unsafe_size(queue));
}

u16 tl_queue_capacity(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(0)
    }

    if (queue->thread_safe) TL_PROFILER_POP_WITH(tl_queue_safe_capacity(queue));
    TL_PROFILER_POP_WITH(tl_queue_unsafe_capacity(queue));
}

b8 tl_queue_is_empty(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(true)
    }

    if (queue->thread_safe) TL_PROFILER_POP_WITH(tl_queue_safe_is_empty(queue));
    TL_PROFILER_POP_WITH(tl_queue_unsafe_is_empty(queue));
}

b8 tl_queue_is_full(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(false)
    }

    if (queue->thread_safe) TL_PROFILER_POP_WITH(tl_queue_safe_is_full(queue));
    TL_PROFILER_POP_WITH(tl_queue_unsafe_is_full(queue));
}

void tl_queue_clear(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    if (queue == NULL) {
        TLWARN("Attempted to use a NULL TLQueue")
        TL_PROFILER_POP
    }

    if (queue->thread_safe) {
        tl_queue_safe_clear(queue);
        TL_PROFILER_POP
    }
    tl_queue_unsafe_clear(queue);
    TL_PROFILER_POP
}

#endif
