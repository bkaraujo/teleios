#ifndef __TELEIOS_CONTAINER_ARRAY__
#define __TELEIOS_CONTAINER_ARRAY__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"
#include "teleios/container/array_safe.inl"
#include "teleios/container/array_unsafe.inl"

// ---------------------------------
// TLArray Implementation
// ---------------------------------

TLArray* tl_array_create(TLAllocator* allocator, u32 initial_capacity, const b8 thread_safe) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, %u", allocator, initial_capacity, thread_safe)

    if (allocator == NULL) {
        TLERROR("Cannot create array with NULL allocator");
        TL_PROFILER_POP_WITH(NULL)
    }

    // Ensure minimum capacity
    if (initial_capacity == 0) {
        initial_capacity = 8;
    }

    TLArray* array = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_ARRAY, sizeof(TLArray));
    if (array == NULL) {
        TLERROR("Failed to allocate TLArray structure");
        TL_PROFILER_POP_WITH(NULL)
    }

    array->capacity = initial_capacity;
    array->count = 0;
    array->mod_count = 0;
    array->allocator = allocator;
    array->thread_safe = thread_safe;

    // Allocate array of void pointers
    array->items = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_ARRAY, sizeof(void*) * initial_capacity);
    if (array->items == NULL) {
        TLERROR("Failed to allocate array items memory");
        tl_memory_free(allocator, array);
        TL_PROFILER_POP_WITH(NULL)
    }

    // Initialize thread-safety primitives
    if (thread_safe) {
        array->mutex = tl_mutex_create(allocator);
        if (array->mutex == NULL) {
            TLERROR("Failed to create array mutex");
            tl_memory_free(allocator, array->items);
            tl_memory_free(allocator, array);
            TL_PROFILER_POP_WITH(NULL)
        }
    }

    TLTRACE("Array created: capacity=%u, thread_safe=%d", initial_capacity, thread_safe);
    TL_PROFILER_POP_WITH(array)
}

void tl_array_destroy(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TL_PROFILER_POP
    }

    TLTRACE("Destroying array: count=%u, capacity=%u", array->count, array->capacity);

    if (array->mutex != NULL) {
        tl_mutex_destroy(array->mutex);
    }

    if (array->items != NULL) {
        tl_memory_free(array->allocator, array->items);
    }

    tl_memory_free(array->allocator, array);
    TL_PROFILER_POP
}


// ---------------------------------
// TLArray Dispatchers
// ---------------------------------

b8 tl_array_push(TLArray* array, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", array, item)

    if (array == NULL) {
        TLWARN("Attempted to push into a NULL TLArray")
        TL_PROFILER_POP_WITH(false)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_push(array, item));
    TL_PROFILER_POP_WITH(tl_array_unsafe_push(array, item));
}

void* tl_array_pop(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TLWARN("Attempted to pop from a NULL TLArray")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (array->count == 0) {
        TL_PROFILER_POP_WITH(NULL)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_pop(array));
    TL_PROFILER_POP_WITH(tl_array_unsafe_pop(array));
}

void* tl_array_get(TLArray* array, const u32 index) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, index)

    if (array == NULL) {
        TLWARN("Attempted to get from a NULL TLArray")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (index >= array->count) {
        TLWARN("Array index %u out of bounds (count=%u)", index, array->count)
        TL_PROFILER_POP_WITH(NULL)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_get(array, index));
    TL_PROFILER_POP_WITH(tl_array_unsafe_get(array, index));
}

b8 tl_array_set(TLArray* array, const u32 index, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)

    if (array == NULL) {
        TLWARN("Attempted to set on a NULL TLArray")
        TL_PROFILER_POP_WITH(false)
    }

    if (index >= array->count) {
        TLWARN("Array index %u out of bounds (count=%u)", index, array->count)
        TL_PROFILER_POP_WITH(false)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_set(array, index, item));
    TL_PROFILER_POP_WITH(tl_array_unsafe_set(array, index, item));
}

b8 tl_array_insert(TLArray* array, const u32 index, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)

    if (array == NULL) {
        TLWARN("Attempted to insert into a NULL TLArray")
        TL_PROFILER_POP_WITH(false)
    }

    if (index > array->count) {
        TLWARN("Array index %u out of bounds for insert (count=%u)", index, array->count)
        TL_PROFILER_POP_WITH(false)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_insert(array, index, item));
    TL_PROFILER_POP_WITH(tl_array_unsafe_insert(array, index, item));
}

void* tl_array_remove(TLArray* array, const u32 index) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, index)

    if (array == NULL) {
        TLWARN("Attempted to remove from a NULL TLArray")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (index >= array->count) {
        TLWARN("Array index %u out of bounds (count=%u)", index, array->count)
        TL_PROFILER_POP_WITH(NULL)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_remove(array, index));
    TL_PROFILER_POP_WITH(tl_array_unsafe_remove(array, index));
}

u32 tl_array_size(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TLWARN("Attempted to get size of a NULL TLArray")
        TL_PROFILER_POP_WITH(0)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_size(array));
    TL_PROFILER_POP_WITH(tl_array_unsafe_size(array));
}

u32 tl_array_capacity(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TLWARN("Attempted to get capacity of a NULL TLArray")
        TL_PROFILER_POP_WITH(0)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_capacity(array));
    TL_PROFILER_POP_WITH(tl_array_unsafe_capacity(array));
}

b8 tl_array_is_empty(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TLWARN("Attempted to check if NULL TLArray is empty")
        TL_PROFILER_POP_WITH(true)
    }

    if (array->thread_safe) TL_PROFILER_POP_WITH(tl_array_safe_is_empty(array));
    TL_PROFILER_POP_WITH(tl_array_unsafe_is_empty(array));
}

void tl_array_clear(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TLWARN("Attempted to clear a NULL TLArray")
        TL_PROFILER_POP
    }

    if (array->thread_safe) {
        tl_array_safe_clear(array);
        TL_PROFILER_POP
    }
    tl_array_unsafe_clear(array);
    TL_PROFILER_POP
}

// ---------------------------------
// Array Iterator Implementation
// ---------------------------------

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
