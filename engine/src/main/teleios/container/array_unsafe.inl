#ifndef __TELEIOS_CONTAINER_ARRAY_UNSAFE__
#define __TELEIOS_CONTAINER_ARRAY_UNSAFE__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

static void tl_array_try_resize(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    const u32 required_capacity = (u32)((f32)array->capacity * 1.75f) + 1;
    if (required_capacity <= array->capacity) TL_PROFILER_POP

    TLDEBUG("Resizing array from %u to %u capacity", array->capacity, required_capacity);

    // Reallocate items array
    void** new_items = tl_memory_alloc(array->allocator, TL_MEMORY_CONTAINER_ARRAY, sizeof(void*) * required_capacity);
    if (new_items == NULL) {
        TLERROR("Failed to reallocate array items");
        TL_PROFILER_POP
    }

    // Copy existing pointers
    if (array->count > 0) {
        tl_memory_copy(new_items, array->items, sizeof(void*) * array->count);
    }

    // Free old memory
    tl_memory_free(array->allocator, array->items);

    array->items = new_items;
    array->capacity = required_capacity;

    TL_PROFILER_POP
}

b8 tl_array_unsafe_push(TLArray* array, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", array, item)
    tl_array_try_resize(array);

    // Store pointer at end of array
    array->items[array->count] = item;
    array->count++;
    array->mod_count++;

    TL_PROFILER_POP_WITH(true)
}

void* tl_array_unsafe_pop(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    // Get last pointer
    void* item = array->items[array->count - 1];

    array->count--;
    array->mod_count++;

    TL_PROFILER_POP_WITH(item)
}

void* tl_array_unsafe_get(TLArray* array, const u32 index) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, index)
    TL_PROFILER_POP_WITH(array->items[index])
}

b8 tl_array_unsafe_set(TLArray* array, const u32 index, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)

    array->items[index] = item;
    array->mod_count++;

    TL_PROFILER_POP_WITH(true)
}

b8 tl_array_unsafe_insert(TLArray* array, const u32 index, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)
    tl_array_try_resize(array);

    // Shift pointers to make room
    if (index < array->count) {
        tl_memory_move(&array->items[index + 1], &array->items[index], sizeof(void*) * (array->count - index));
    }

    // Store new pointer
    array->items[index] = item;
    array->count++;
    array->mod_count++;

    TL_PROFILER_POP_WITH(true)
}

void* tl_array_unsafe_remove(TLArray* array, const u32 index) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, index)

    // Get pointer to remove
    void* item = array->items[index];

    // Shift pointers to fill gap
    if (index < array->count - 1) {
        tl_memory_move(&array->items[index], &array->items[index + 1], sizeof(void*) * (array->count - index - 1));
    }

    array->count--;
    array->mod_count++;

    TL_PROFILER_POP_WITH(item)
}

u32 tl_array_unsafe_size(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    TL_PROFILER_POP_WITH(array->count)
}

u32 tl_array_unsafe_capacity(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    TL_PROFILER_POP_WITH(array->capacity)
}

b8 tl_array_unsafe_is_empty(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    TL_PROFILER_POP_WITH(array->count == 0)
}

void tl_array_unsafe_clear(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)

    array->count = 0;
    array->mod_count++;

    TL_PROFILER_POP
}

#endif
