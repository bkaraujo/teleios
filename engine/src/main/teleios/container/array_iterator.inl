#ifndef __TELEIOS_CONTAINER_ARRAY_ITERATOR__
#define __TELEIOS_CONTAINER_ARRAY_ITERATOR__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

typedef struct {
    u32 current_index;  // Current position in array
} TLArrayIteratorState;

static void tl_array_iterator_check_modification(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLArray* array = (const TLArray*)iterator->source;

    if (array->thread_safe) tl_mutex_lock(array->mutex);
    const u32 current_mod_count = array->mod_count;
    if (array->thread_safe) tl_mutex_unlock(array->mutex);

    if (current_mod_count != iterator->expected_mod_count) {
        TLFATAL("Concurrent modification detected during iteration (expected=%u, actual=%u)",
                iterator->expected_mod_count, current_mod_count);
    }

    TL_PROFILER_POP
}

static b8 tl_array_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLArrayIteratorState* state = (const TLArrayIteratorState*)iterator->state;
    const b8 has_next = state->current_index < iterator->size;

    TL_PROFILER_POP_WITH(has_next)
}

static void* tl_array_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLArrayIteratorState* state = (TLArrayIteratorState*)iterator->state;

    if (state->current_index >= iterator->size) {
        TLWARN("Iterator exhausted");
        TL_PROFILER_POP_WITH(NULL)
    }

    const TLArray* array = (const TLArray*)iterator->source;

    // Return pointer at current index
    if (array->thread_safe) tl_mutex_lock(array->mutex);
    void* item = array->items[state->current_index];
    if (array->thread_safe) tl_mutex_unlock(array->mutex);

    state->current_index++;

    TL_PROFILER_POP_WITH(item)
}

static void tl_array_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLArrayIteratorState* state = (TLArrayIteratorState*)iterator->state;
    state->current_index = 0;

    TL_PROFILER_POP
}

static void tl_array_iterator_resync(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLArray* array = (const TLArray*)iterator->source;

    if (array->thread_safe) tl_mutex_lock(array->mutex);
    iterator->expected_mod_count = array->mod_count;
    iterator->size = array->count;
    if (array->thread_safe) tl_mutex_unlock(array->mutex);

    // Rewind to beginning
    tl_array_iterator_rewind(iterator);

    TL_PROFILER_POP
}

TLIterator* tl_array_iterator(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TLERROR("Attempted to use a NULL TLArray");
        TL_PROFILER_POP_WITH(NULL)
    }

    // Lock array to capture current state
    if (array->thread_safe) tl_mutex_lock(array->mutex);

    // Allocate iterator on array's allocator
    TLIterator* iterator = tl_memory_alloc(array->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));

    // Allocate state on array's allocator
    TLArrayIteratorState* state = tl_memory_alloc(array->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLArrayIteratorState));

    // Initialize state
    state->current_index = 0;

    // Initialize fail-fast iterator
    iterator->source = array;
    iterator->expected_mod_count = array->mod_count;
    iterator->size = array->count;
    iterator->state = state;
    iterator->allocator = array->allocator;

    // Assign function pointers
    iterator->has_modified = tl_array_iterator_check_modification;
    iterator->has_next = tl_array_iterator_has_next;
    iterator->next = tl_array_iterator_next;
    iterator->rewind = tl_array_iterator_rewind;
    iterator->resync = tl_array_iterator_resync;

    if (array->thread_safe) tl_mutex_unlock(array->mutex);

    TL_PROFILER_POP_WITH(iterator)
}


#endif
