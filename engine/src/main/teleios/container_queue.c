#include "memory_types.inl"
#include "teleios/teleios.h"
#include "teleios/container_types.inl"

// ---------------------------------
// Queue Lifecycle
// ---------------------------------

TLQueue* tl_queue_create(TLAllocator* allocator, const u16 capacity) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", allocator, capacity)

    if (allocator == NULL) {
        TLERROR("Attempt to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (capacity == 0) {
        TLERROR("Attempt to use a capacity of 0")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLQueue* queue = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_QUEUE, sizeof(TLQueue));
    queue->items = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_QUEUE, sizeof(void*) * capacity);

    queue->capacity = capacity;
    queue->allocator = allocator;

    queue->mutex = tl_mutex_create(allocator);
    queue->not_empty = tl_condition_create(allocator);
    queue->not_full = tl_condition_create(allocator);

    if (!queue->mutex || !queue->not_empty || !queue->not_full) {
        TLFATAL("Failed to create synchronization primitives for queue")
    }

    TL_PROFILER_POP_WITH(queue)
}

void tl_queue_destroy(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP
    }

    if (queue->mutex) tl_mutex_destroy(queue->mutex);
    if (queue->not_empty) tl_condition_destroy(queue->not_empty);
    if (queue->not_full) tl_condition_destroy(queue->not_full);
    tl_memory_free(queue->allocator, queue->items);
    tl_memory_free(queue->allocator, queue);

    TL_PROFILER_POP
}

// ---------------------------------
// Queue Operations
// ---------------------------------

void tl_queue_offer(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP
    }

    tl_mutex_lock(queue->mutex);

    if (queue->count >= queue->capacity) {
        tl_mutex_unlock(queue->mutex);
        TLWARN("Queue is full, cannot offer payload")
        TL_PROFILER_POP
    }

    queue->items[queue->head] = payload;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count++;

    tl_condition_signal(queue->not_empty);
    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP
}

void tl_queue_push(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP
    }

    tl_mutex_lock(queue->mutex);

    // Wait for space if queue is full (blocking)
    while (queue->count >= queue->capacity) {
        TLWARN("Queue 0x%p waiting for available slot. Capacity %d", queue, queue->capacity);
        tl_condition_wait(queue->not_full, queue->mutex);
    }

    queue->items[queue->head] = payload;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count++;

    tl_condition_signal(queue->not_empty);
    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP
}

void* tl_queue_pop(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(queue->mutex);

    if (queue->count == 0) {
        tl_mutex_unlock(queue->mutex);
        TL_PROFILER_POP_WITH(NULL)
    }

    void* payload = queue->items[queue->tail];
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count--;

    tl_condition_signal(queue->not_full);
    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP_WITH(payload)
}

void* tl_queue_peek(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(queue->mutex);

    void* payload = NULL;
    if (queue->count > 0) {
        payload = queue->items[queue->tail];
    }

    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP_WITH(payload)
}

// ---------------------------------
// Queue Queries
// ---------------------------------

u16 tl_queue_size(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(0)
    }

    tl_mutex_lock(queue->mutex);
    const u16 size = queue->count;
    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP_WITH(size)
}

u16 tl_queue_capacity(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(0)
    }
    TL_PROFILER_POP_WITH(queue->capacity)
}

b8 tl_queue_is_empty(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(true)
    }

    tl_mutex_lock(queue->mutex);
    const b8 empty = (queue->count == 0);
    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP_WITH(empty)
}

b8 tl_queue_is_full(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(queue->mutex);
    const b8 full = (queue->count >= queue->capacity);
    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP_WITH(full)
}

void tl_queue_clear(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    if (queue == NULL) {
        TLERROR("Attempt to use a NULL TLQueue")
        TL_PROFILER_POP
    }

    tl_mutex_lock(queue->mutex);

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    // Signal waiting threads
    tl_condition_broadcast(queue->not_full);
    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP
}