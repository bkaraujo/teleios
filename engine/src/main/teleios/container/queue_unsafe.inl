#ifndef __TELEIOS_CONTAINER_QUEUE_UNSAFE__
#define __TELEIOS_CONTAINER_QUEUE_UNSAFE__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

void tl_queue_unsafe_offer(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)

    queue->items[queue->head] = payload;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count++;
    queue->mod_count++;

    TL_PROFILER_POP
}

void tl_queue_unsafe_push(TLQueue* queue, void* payload) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, payload)

    queue->items[queue->head] = payload;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count++;
    queue->mod_count++;

    TL_PROFILER_POP
}

void* tl_queue_unsafe_pop(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    void* payload = queue->items[queue->tail];
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count--;
    queue->mod_count++;

    TL_PROFILER_POP_WITH(payload)
}

void* tl_queue_unsafe_peek(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    TL_PROFILER_POP_WITH(queue->items[queue->tail])
}

u16 tl_queue_unsafe_size(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    TL_PROFILER_POP_WITH(queue->count)
}

u16 tl_queue_unsafe_capacity(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    TL_PROFILER_POP_WITH(queue->capacity)
}

b8 tl_queue_unsafe_is_empty(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    TL_PROFILER_POP_WITH(queue->count == 0)
}

b8 tl_queue_unsafe_is_full(const TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)
    TL_PROFILER_POP_WITH(queue->count >= queue->capacity)
}

void tl_queue_unsafe_clear(TLQueue* queue) {
    TL_PROFILER_PUSH_WITH("0x%p", queue)

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->mod_count++;

    TL_PROFILER_POP
}

#endif
