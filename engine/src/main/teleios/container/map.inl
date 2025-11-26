#ifndef __TELEIOS_CONTAINER_MAP__
#define __TELEIOS_CONTAINER_MAP__

#include "teleios/memory/types.inl"
#include "teleios/container/types.inl"
#include "teleios/teleios.h"
#include "teleios/container/map_safe.inl"
#include "teleios/container/map_unsafe.inl"

// ---------------------------------
// TLMap Implementation
// ---------------------------------

TLMap* tl_map_create(TLAllocator* allocator, const u32 capacity, const b8 thread_safe) {
    TL_PROFILER_PUSH_WITH("0x%p, %u, %d", allocator, capacity, thread_safe)

    if (allocator == NULL) {
        TLERROR("Attempted to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLMap* map = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_MAP, sizeof(TLMap));

    const u32 actual_capacity = tl_number_next_power_of_2(capacity == 0 ? 16 : capacity);
    map->buckets = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_MAP, sizeof(TLMapEntry*) * actual_capacity);
    map->capacity = actual_capacity;
    map->size = 0;
    map->mod_count = 0;
    map->load_factor = 0.75f;
    map->allocator = allocator;
    map->thread_safe = thread_safe;
    map->mutex = NULL;

    if (thread_safe) {
        map->mutex = tl_mutex_create(allocator);
        if (!map->mutex) {
            TLERROR("Failed to create mutex for map")
            tl_memory_free(allocator, map->buckets);
            tl_memory_free(allocator, map);
            TL_PROFILER_POP_WITH(NULL)
        }
    }

    TL_PROFILER_POP_WITH(map)
}

void tl_map_destroy(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLERROR("Attempted to destroy a NULL TLMap")
        TL_PROFILER_POP
    }

    tl_map_clear(map);

    if (map->mutex) tl_mutex_destroy(map->mutex);
    tl_memory_free(map->allocator, map->buckets);
    tl_memory_free(map->allocator, map);

    TL_PROFILER_POP
}

// ---------------------------------
// TLMap Dispatchers
// ---------------------------------

TLList* tl_map_get(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TLERROR("Attempted to get from a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLERROR("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_get(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_get(map, key));
}

TLList* tl_map_get_or_create(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TLERROR("Attempted to get_or_create from a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLERROR("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_get_or_create(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_get_or_create(map, key));
}

void tl_map_put(TLMap* map, TLString* key, void* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", map, key, value)

    if (map == NULL) {
        TLERROR("Attempted to put into a NULL TLMap")
        TL_PROFILER_POP
    }

    if (key == NULL) {
        TLERROR("Attempted to use a NULL TLString")
        TL_PROFILER_POP
    }

    if (map->thread_safe) {
        tl_map_safe_put(map, key, value);
        TL_PROFILER_POP
    }
    tl_map_unsafe_put(map, key, value);
    TL_PROFILER_POP
}

b8 tl_map_contains(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TLERROR("Attempted to read a NULL TLMap")
        TL_PROFILER_POP_WITH(false)
    }

    if (key == NULL) {
        TLERROR("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(false)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_contains(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_contains(map, key));
}

TLList* tl_map_remove(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TLERROR("Attempted to remove a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLERROR("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_remove(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_remove(map, key));
}

u32 tl_map_size(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLERROR("Attempted to read a NULL TLMap")
        TL_PROFILER_POP_WITH(0)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_size(map));
    TL_PROFILER_POP_WITH(tl_map_unsafe_size(map));
}

u32 tl_map_capacity(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLERROR("Attempted to read a NULL TLMap")
        TL_PROFILER_POP_WITH(0)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_capacity(map));
    TL_PROFILER_POP_WITH(tl_map_unsafe_capacity(map));
}

b8 tl_map_is_empty(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLERROR("Attempted to read a NULL TLMap")
        TL_PROFILER_POP_WITH(true)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_is_empty(map));
    TL_PROFILER_POP_WITH(tl_map_unsafe_is_empty(map));
}

void tl_map_clear(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLERROR("Attempted to clear a NULL TLMap")
        TL_PROFILER_POP
    }

    if (map->thread_safe) {
        tl_map_safe_clear(map);
        TL_PROFILER_POP
    }
    tl_map_unsafe_clear(map);
    TL_PROFILER_POP
}

#endif
