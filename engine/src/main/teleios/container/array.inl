#ifndef __TELEIOS_CONTAINER_ARRAY__
#define __TELEIOS_CONTAINER_ARRAY__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

// ---------------------------------
// TLArray Implementation
// ---------------------------------

TLArray* tl_array_create(TLAllocator* allocator, const u32 stride, u32 initial_capacity) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, %u", allocator, stride, initial_capacity)

    if (allocator == NULL) {
        TLERROR("Cannot create array with NULL allocator");
        TL_PROFILER_POP_WITH(NULL)
    }

    if (stride == 0) {
        TLERROR("Cannot create array with stride 0");
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

    array->stride = stride;
    array->count = 0;
    array->capacity = initial_capacity;
    array->mod_count = 0;
    array->allocator = allocator;

    // Allocate contiguous memory block
    array->items = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_ARRAY, stride * initial_capacity);
    if (array->items == NULL) {
        TLERROR("Failed to allocate array items memory");
        tl_memory_free(allocator, array);
        TL_PROFILER_POP_WITH(NULL)
    }

    // Initialize thread-safety primitives
    array->mutex = tl_mutex_create(allocator);
    if (array->mutex == NULL) {
        TLERROR("Failed to create array mutex");
        tl_memory_free(allocator, array->items);
        tl_memory_free(allocator, array);
        TL_PROFILER_POP_WITH(NULL)
    }

    TLTRACE("Array created: stride=%u, capacity=%u", stride, initial_capacity);
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

static b8 tl_array_ensure_capacity(TLArray* array, const u32 required_capacity) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, required_capacity)
    if (required_capacity <= array->capacity) {
        return true;
    }

    TLDEBUG("Resizing array from %u to %u capacity", array->capacity, required_capacity);

    // Reallocate items
    void* new_items = tl_memory_alloc(array->allocator, TL_MEMORY_CONTAINER_ARRAY, array->stride * required_capacity);
    if (new_items == NULL) {
        TLERROR("Failed to reallocate array items");
        TL_PROFILER_POP_WITH(false)
    }

    // Copy existing data
    if (array->count > 0) {
        tl_memory_copy(new_items, array->items, array->stride * array->count);
    }

    // Free old memory
    tl_memory_free(array->allocator, array->items);

    array->items = new_items;
    array->capacity = required_capacity;

    TL_PROFILER_POP_WITH(true)
}

b8 tl_array_push(TLArray* array, const void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", array, item)

    if (array == NULL || item == NULL) {
        TLWARN("Cannot push to NULL array or NULL item");
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(array->mutex);

    // Ensure capacity
    if (!tl_array_ensure_capacity(array, (u32)((f32)array->count * 1.75f) + 1)) {
        tl_mutex_unlock(array->mutex);
        TL_PROFILER_POP_WITH(false)
    }

    // Copy item to end of array
    u8* dest = (u8*)array->items + (array->stride * array->count);
    tl_memory_copy(dest, item, array->stride);
    array->count++;
    array->mod_count++;

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(true)
}

b8 tl_array_pop(TLArray* array, void* out_item) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", array, out_item)

    if (array == NULL) {
        TLWARN("Cannot pop from NULL array");
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(array->mutex);

    if (array->count == 0) {
        tl_mutex_unlock(array->mutex);
        TL_PROFILER_POP_WITH(false)
    }

    // Copy last item to out_item if provided
    if (out_item != NULL) {
        const u8* src = (const u8*)array->items + (array->stride * (array->count - 1));
        tl_memory_copy(out_item, src, array->stride);
    }

    array->count--;
    array->mod_count++;

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(true)
}

void* tl_array_get(TLArray* array, const u32 index) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, index)

    if (array == NULL) {
        TLWARN("Cannot get from NULL array");
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(array->mutex);

    if (index >= array->count) {
        tl_mutex_unlock(array->mutex);
        TLWARN("Array index %u out of bounds (count=%u)", index, array->count);
        TL_PROFILER_POP_WITH(NULL)
    }

    u8* item = (u8*)array->items + (array->stride * index);
    tl_mutex_unlock(array->mutex);

    TL_PROFILER_POP_WITH(item)
}

b8 tl_array_set(TLArray* array, const u32 index, const void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)

    if (array == NULL || item == NULL) {
        TLWARN("Cannot set in NULL array or NULL item");
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(array->mutex);

    if (index >= array->count) {
        tl_mutex_unlock(array->mutex);
        TLWARN("Array index %u out of bounds (count=%u)", index, array->count);
        TL_PROFILER_POP_WITH(false)
    }

    u8* dest = (u8*)array->items + (array->stride * index);
    tl_memory_copy(dest, item, array->stride);
    array->mod_count++;

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(true)
}

b8 tl_array_insert(TLArray* array, const u32 index, const void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)

    if (array == NULL || item == NULL) {
        TLWARN("Cannot insert into NULL array or NULL item");
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(array->mutex);

    if (index > array->count) {
        tl_mutex_unlock(array->mutex);
        TLWARN("Array index %u out of bounds for insert (count=%u)", index, array->count);
        TL_PROFILER_POP_WITH(false)
    }

    // Ensure capacity
    if (!tl_array_ensure_capacity(array, (u32)((f32)array->count * 1.75f) + 1)) {
        tl_mutex_unlock(array->mutex);
        TL_PROFILER_POP_WITH(false)
    }

    // Shift elements to make room
    if (index < array->count) {
        u8* src = (u8*)array->items + (array->stride * index);
        u8* dest = src + array->stride;
        const u32 bytes_to_move = array->stride * (array->count - index);
        tl_memory_move(dest, src, bytes_to_move);
    }

    // Copy new item
    u8* dest = (u8*)array->items + (array->stride * index);
    tl_memory_copy(dest, item, array->stride);
    array->count++;
    array->mod_count++;

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(true)
}

b8 tl_array_remove(TLArray* array, const u32 index, void* out_item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, out_item)

    if (array == NULL) {
        TLWARN("Cannot remove from NULL array");
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(array->mutex);

    if (index >= array->count) {
        tl_mutex_unlock(array->mutex);
        TLWARN("Array index %u out of bounds (count=%u)", index, array->count);
        TL_PROFILER_POP_WITH(false)
    }

    // Copy item if requested
    if (out_item != NULL) {
        const u8* src = (const u8*)array->items + (array->stride * index);
        tl_memory_copy(out_item, src, array->stride);
    }

    // Shift elements to fill gap
    if (index < array->count - 1) {
        u8* dest = (u8*)array->items + (array->stride * index);
        u8* src = dest + array->stride;
        u32 bytes_to_move = array->stride * (array->count - index - 1);
        tl_memory_move(dest, src, bytes_to_move);
    }

    array->count--;
    array->mod_count++;

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(true)
}

u32 tl_array_size(const TLArray* array) {
    if (array == NULL) return 0;

    tl_mutex_lock(array->mutex);
    const u32 size = array->count;
    tl_mutex_unlock(array->mutex);

    return size;
}

u32 tl_array_capacity(const TLArray* array) {
    if (array == NULL) return 0;

    tl_mutex_lock(array->mutex);
    const u32 capacity = array->capacity;
    tl_mutex_unlock(array->mutex);

    return capacity;
}

b8 tl_array_is_empty(const TLArray* array) {
    return tl_array_size(array) == 0;
}

void tl_array_clear(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL) {
        TL_PROFILER_POP
    }

    tl_mutex_lock(array->mutex);
    array->count = 0;
    array->mod_count++;
    tl_mutex_unlock(array->mutex);

    TL_PROFILER_POP
}

b8 tl_array_reserve(TLArray* array, const u32 capacity) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, capacity)

    if (array == NULL) {
        TLWARN("Cannot reserve capacity for NULL array");
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(array->mutex);
    const b8 result = tl_array_ensure_capacity(array, capacity);
    tl_mutex_unlock(array->mutex);

    TL_PROFILER_POP_WITH(result)
}

void tl_array_shrink_to_fit(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    if (array == NULL || array->count == array->capacity) {
        TL_PROFILER_POP
    }

    tl_mutex_lock(array->mutex);

    if (array->count == 0) {
        // Keep minimum capacity of 8
        array->capacity = 8;
    } else {
        array->capacity = array->count;
    }

    const u32 new_capacity = array->capacity;

    // Reallocate to exact size
    void* new_items = tl_memory_alloc(array->allocator, TL_MEMORY_CONTAINER_ARRAY, array->stride * new_capacity);
    if (new_items != NULL) {
        if (array->count > 0) {
            tl_memory_copy(new_items, array->items, array->stride * array->count);
        }
        tl_memory_free(array->allocator, array->items);
        array->items = new_items;
        TLDEBUG("Array shrunk to capacity %u", array->capacity);
    } else {
        TLWARN("Failed to shrink array");
    }

    tl_mutex_unlock(array->mutex);
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

    tl_mutex_lock(array->mutex);
    const u32 current_mod_count = array->mod_count;
    tl_mutex_unlock(array->mutex);

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

    // Return pointer to item at current index
    tl_mutex_lock(array->mutex);
    u8* item = (u8*)array->items + (array->stride * state->current_index);
    tl_mutex_unlock(array->mutex);

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

    tl_mutex_lock(array->mutex);
    iterator->expected_mod_count = array->mod_count;
    iterator->size = array->count;
    tl_mutex_unlock(array->mutex);

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
    tl_mutex_lock(array->mutex);

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

    tl_mutex_unlock(array->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

#endif
