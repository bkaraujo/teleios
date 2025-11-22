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
    if (map == NULL) {
        TLERROR("Failed to allocate TLMap structure")
        TL_PROFILER_POP_WITH(NULL)
    }

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

    TLTRACE("Map created: thread_safe=%d, capacity=%u", thread_safe, actual_capacity);
    TL_PROFILER_POP_WITH(map)
}

void tl_map_destroy(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TL_PROFILER_POP
    }

    TLTRACE("Destroying map: size=%u", map->size);

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
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLWARN("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_get(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_get(map, key));
}

TLList* tl_map_get_or_create(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLWARN("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_get_or_create(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_get_or_create(map, key));
}

void tl_map_put(TLMap* map, TLString* key, void* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", map, key, value)

    if (map == NULL) {
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP
    }

    if (key == NULL) {
        TLWARN("Attempted to use a NULL TLString")
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
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(false)
    }

    if (key == NULL) {
        TLWARN("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(false)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_contains(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_contains(map, key));
}

TLList* tl_map_remove(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLWARN("Attempted to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_remove(map, key));
    TL_PROFILER_POP_WITH(tl_map_unsafe_remove(map, key));
}

u32 tl_map_size(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(0)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_size(map));
    TL_PROFILER_POP_WITH(tl_map_unsafe_size(map));
}

u32 tl_map_capacity(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(0)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_capacity(map));
    TL_PROFILER_POP_WITH(tl_map_unsafe_capacity(map));
}

b8 tl_map_is_empty(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(true)
    }

    if (map->thread_safe) TL_PROFILER_POP_WITH(tl_map_safe_is_empty(map));
    TL_PROFILER_POP_WITH(tl_map_unsafe_is_empty(map));
}

void tl_map_clear(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLWARN("Attempted to use a NULL TLMap")
        TL_PROFILER_POP
    }

    if (map->thread_safe) {
        tl_map_safe_clear(map);
        TL_PROFILER_POP
    }
    tl_map_unsafe_clear(map);
    TL_PROFILER_POP
}

// ---------------------------------
// Map Iterator Implementation
// ---------------------------------

typedef struct {
    u32 bucket_index;
    TLMapEntry* current_entry;
} TLMapIteratorState;

static void tl_map_iterator_check_modification(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLMap* map = (const TLMap*)iterator->source;

    if (map->thread_safe) tl_mutex_lock(map->mutex);
    const u32 current_mod_count = map->mod_count;
    if (map->thread_safe) tl_mutex_unlock(map->mutex);

    if (current_mod_count != iterator->expected_mod_count) {
        TLFATAL("Concurrent modification detected during map iteration (expected=%u, actual=%u)",
                iterator->expected_mod_count, current_mod_count)
    }

    TL_PROFILER_POP
}

static b8 tl_map_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLMapIteratorState* state = (const TLMapIteratorState*)iterator->state;

    TL_PROFILER_POP_WITH(state->current_entry != NULL)
}

static void* tl_map_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLMapIteratorState* state = (TLMapIteratorState*)iterator->state;

    if (state->current_entry == NULL) {
        TLWARN("Iterator exhausted")
        TL_PROFILER_POP_WITH(NULL)
    }

    void* key = (void*)state->current_entry->key;

    state->current_entry = state->current_entry->next;

    if (state->current_entry == NULL) {
        TLMap* map = (TLMap*)iterator->source;
        state->bucket_index++;

        while (state->bucket_index < map->capacity) {
            if (map->buckets[state->bucket_index] != NULL) {
                state->current_entry = map->buckets[state->bucket_index];
                break;
            }
            state->bucket_index++;
        }
    }

    TL_PROFILER_POP_WITH(key)
}

static void tl_map_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLMap* map = (const TLMap*)iterator->source;
    TLMapIteratorState* state = (TLMapIteratorState*)iterator->state;

    if (map->thread_safe) tl_mutex_lock(map->mutex);

    state->bucket_index = 0;
    state->current_entry = NULL;

    for (u32 i = 0; i < map->capacity; ++i) {
        if (map->buckets[i] != NULL) {
            state->bucket_index = i;
            state->current_entry = map->buckets[i];
            break;
        }
    }

    if (map->thread_safe) tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP
}

static void tl_map_iterator_resync(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLMap* map = (TLMap*)iterator->source;
    TLMapIteratorState* state = (TLMapIteratorState*)iterator->state;

    if (map->thread_safe) tl_mutex_lock(map->mutex);

    iterator->expected_mod_count = map->mod_count;
    iterator->size = map->size;

    state->bucket_index = 0;
    state->current_entry = NULL;

    for (u32 i = 0; i < map->capacity; ++i) {
        if (map->buckets[i] != NULL) {
            state->bucket_index = i;
            state->current_entry = map->buckets[i];
            break;
        }
    }

    if (map->thread_safe) tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP
}

TLIterator* tl_map_keys(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLERROR("Attempted to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (map->thread_safe) tl_mutex_lock(map->mutex);

    TLIterator* iterator = tl_memory_alloc(map->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));
    TLMapIteratorState* state = tl_memory_alloc(map->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLMapIteratorState));

    state->bucket_index = 0;
    state->current_entry = NULL;

    for (u32 i = 0; i < map->capacity; ++i) {
        if (map->buckets[i] != NULL) {
            state->bucket_index = i;
            state->current_entry = map->buckets[i];
            break;
        }
    }

    iterator->source = map;
    iterator->expected_mod_count = map->mod_count;
    iterator->size = map->size;
    iterator->state = state;
    iterator->allocator = map->allocator;

    iterator->has_modified = tl_map_iterator_check_modification;
    iterator->has_next = tl_map_iterator_has_next;
    iterator->next = tl_map_iterator_next;
    iterator->rewind = tl_map_iterator_rewind;
    iterator->resync = tl_map_iterator_resync;

    if (map->thread_safe) tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

#endif
