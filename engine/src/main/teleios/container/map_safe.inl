#ifndef __TELEIOS_CONTAINER_MAP_SAFE__
#define __TELEIOS_CONTAINER_MAP_SAFE__

#include "teleios/teleios.h"
#include "teleios/container/map_unsafe.inl"

TLList* tl_map_safe_get(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)
    tl_mutex_lock(map->mutex);
    TLList* result = tl_map_unsafe_get(map, key);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(result)
}

TLList* tl_map_safe_get_or_create(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)
    tl_mutex_lock(map->mutex);
    TLList* result = tl_map_unsafe_get_or_create(map, key);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(result)
}

void tl_map_safe_put(TLMap* map, TLString* key, void* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", map, key, value)
    tl_mutex_lock(map->mutex);
    tl_map_unsafe_put(map, key, value);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP
}

b8 tl_map_safe_contains(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)
    tl_mutex_lock(map->mutex);
    const b8 result = tl_map_unsafe_contains(map, key);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(result)
}

TLList* tl_map_safe_remove(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)
    tl_mutex_lock(map->mutex);
    TLList* result = tl_map_unsafe_remove(map, key);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(result)
}

u32 tl_map_safe_size(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    tl_mutex_lock(map->mutex);
    const u32 result = tl_map_unsafe_size(map);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(result)
}

u32 tl_map_safe_capacity(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    tl_mutex_lock(map->mutex);
    const u32 result = tl_map_unsafe_capacity(map);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_map_safe_is_empty(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    tl_mutex_lock(map->mutex);
    const b8 result = tl_map_unsafe_is_empty(map);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(result)
}

void tl_map_safe_clear(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    tl_mutex_lock(map->mutex);
    tl_map_unsafe_clear(map);
    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP
}

#endif
