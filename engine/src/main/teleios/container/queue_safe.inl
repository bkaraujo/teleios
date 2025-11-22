#ifndef __TELEIOS_CONTAINER_QUEUE_SAFE__
#define __TELEIOS_CONTAINER_QUEUE_SAFE__

#include "teleios/teleios.h"
#include "teleios/container/queue_unsafe.inl"

void tl_queue_safe_offer(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)
    tl_mutex_lock(queue->mutex);

    tl_queue_unsafe_offer(queue, payload);
    tl_condition_signal(queue->not_empty);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP
}

void tl_queue_safe_push(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)
    tl_mutex_lock(queue->mutex);

    // Wait for space if queue is full (blocking)
    while (queue->count >= queue->capacity) {
        TLWARN("Queue 0x%p waiting for available slot. Capacity %d", queue, queue->capacity);
        tl_condition_wait(queue->not_full, queue->mutex);
    }

    tl_queue_unsafe_push(queue, payload);
    tl_condition_signal(queue->not_empty);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP
}

void* tl_queue_safe_pop(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    tl_mutex_lock(queue->mutex);

    void* result = tl_queue_unsafe_pop(queue);
    tl_condition_signal(queue->not_full);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_queue_safe_peek(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    tl_mutex_lock(queue->mutex);

    void* result = tl_queue_unsafe_peek(queue);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP_WITH(result)
}

u16 tl_queue_safe_size(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    tl_mutex_lock(queue->mutex);

    const u16 result = tl_queue_unsafe_size(queue);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP_WITH(result)
}

u16 tl_queue_safe_capacity(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    tl_mutex_lock(queue->mutex);

    const u16 result = tl_queue_unsafe_capacity(queue);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_queue_safe_is_empty(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    tl_mutex_lock(queue->mutex);

    const b8 result = tl_queue_unsafe_is_empty(queue);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_queue_safe_is_full(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    tl_mutex_lock(queue->mutex);

    const b8 result = tl_queue_unsafe_is_full(queue);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP_WITH(result)
}

void tl_queue_safe_clear(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    tl_mutex_lock(queue->mutex);

    tl_queue_unsafe_clear(queue);
    tl_condition_broadcast(queue->not_full);

    tl_mutex_unlock(queue->mutex);
    TL_PROFILER_POP
}

#endif
