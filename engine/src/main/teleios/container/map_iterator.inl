#ifndef __TELEIOS_CONTAINER_MAP_ITERATOR__
#define __TELEIOS_CONTAINER_MAP_ITERATOR__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

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
