#ifndef __TELEIOS_CONTAINER_ARRAY_SAFE__
#define __TELEIOS_CONTAINER_ARRAY_SAFE__

#include "teleios/teleios.h"
#include "teleios/container/array_unsafe.inl"

b8 tl_array_safe_push(TLArray* array, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", array, item)
    tl_mutex_lock(array->mutex);

    const b8 result = tl_array_unsafe_push(array, item);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_array_safe_pop(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    tl_mutex_lock(array->mutex);

    void* result = tl_array_unsafe_pop(array);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_array_safe_get(TLArray* array, const u32 index) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, index)
    tl_mutex_lock(array->mutex);

    void* result = tl_array_unsafe_get(array, index);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_array_safe_set(TLArray* array, const u32 index, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)
    tl_mutex_lock(array->mutex);

    const b8 result = tl_array_unsafe_set(array, index, item);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_array_safe_insert(TLArray* array, const u32 index, void* item) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, 0x%p", array, index, item)
    tl_mutex_lock(array->mutex);

    const b8 result = tl_array_unsafe_insert(array, index, item);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

void* tl_array_safe_remove(TLArray* array, const u32 index) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", array, index)
    tl_mutex_lock(array->mutex);

    void* result = tl_array_unsafe_remove(array, index);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

u32 tl_array_safe_size(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    tl_mutex_lock(array->mutex);

    const u32 result = tl_array_unsafe_size(array);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

u32 tl_array_safe_capacity(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    tl_mutex_lock(array->mutex);

    const u32 result = tl_array_unsafe_capacity(array);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_array_safe_is_empty(const TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    tl_mutex_lock(array->mutex);

    const b8 result = tl_array_unsafe_is_empty(array);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP_WITH(result)
}

void tl_array_safe_clear(TLArray* array) {
    TL_PROFILER_PUSH_WITH("0x%p", array)
    tl_mutex_lock(array->mutex);

    tl_array_unsafe_clear(array);

    tl_mutex_unlock(array->mutex);
    TL_PROFILER_POP
}

#endif
