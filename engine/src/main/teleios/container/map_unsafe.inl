#ifndef __TELEIOS_CONTAINER_MAP_UNSAFE__
#define __TELEIOS_CONTAINER_MAP_UNSAFE__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

// ---------------------------------
// Internal Helper Functions
// ---------------------------------

static u32 tl_map_hash(const TLString* key, const u32 capacity) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", key, capacity)

    if (key == NULL) {
        TL_PROFILER_POP_WITH(0)
    }

    const char* str = tl_string_cstr(key);
    const u32 len = tl_string_length(key);

    // FNV-1a hash
    u32 hash = 2166136261u;
    for (u32 i = 0; i < len; i++) {
        hash ^= (u8)str[i];
        hash *= 16777619u;
    }

    const u32 index = hash % capacity;
    TL_PROFILER_POP_WITH(index)
}

static TLMapEntry* tl_map_create_entry(TLAllocator* allocator, const TLString* key, TLList* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", allocator, key, value)

    TLMapEntry* entry = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_MAP, sizeof(TLMapEntry));
    entry->key = tl_string_copy(key);
    entry->value = value;

    TL_PROFILER_POP_WITH(entry)
}

static void tl_map_free_entry(TLAllocator* allocator, TLMapEntry* entry) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", allocator, entry)

    if (entry == NULL) {
        TL_PROFILER_POP
    }

    if (entry->key) tl_string_destroy(entry->key);
    if (entry->value) tl_list_destroy(entry->value);

    tl_memory_free(allocator, entry);

    TL_PROFILER_POP
}

// ---------------------------------
// Map Operations (Unsafe)
// ---------------------------------

TLList* tl_map_unsafe_get(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    const u32 index = tl_map_hash(key, map->capacity);
    const TLMapEntry* entry = map->buckets[index];

    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            TL_PROFILER_POP_WITH(entry->value)
        }
        entry = entry->next;
    }

    TL_PROFILER_POP_WITH(NULL)
}

TLList* tl_map_unsafe_get_or_create(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    const u32 index = tl_map_hash(key, map->capacity);
    TLMapEntry* entry = map->buckets[index];

    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            TL_PROFILER_POP_WITH(entry->value)
        }
        entry = entry->next;
    }

    // Key doesn't exist - create new entry
    // Internal list is non-thread-safe (map handles thread safety)
    TLList* new_list = tl_list_create(map->allocator, false);
    TLMapEntry* new_entry = tl_map_create_entry(map->allocator, key, new_list);

    if (new_entry == NULL) {
        tl_list_destroy(new_list);
        TLERROR("Failed to create map entry")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Insert at head of bucket chain
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->size++;
    map->mod_count++;

    TL_PROFILER_POP_WITH(new_list)
}

void tl_map_unsafe_put(TLMap* map, TLString* key, void* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", map, key, value)

    TLList* list = tl_map_unsafe_get_or_create(map, key);
    if (list != NULL) {
        tl_list_push_back(list, value);
    }

    tl_string_destroy(key);

    TL_PROFILER_POP
}

b8 tl_map_unsafe_contains(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    const u32 index = tl_map_hash(key, map->capacity);
    TLMapEntry* entry = map->buckets[index];

    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            TL_PROFILER_POP_WITH(true)
        }
        entry = entry->next;
    }

    TL_PROFILER_POP_WITH(false)
}

TLList* tl_map_unsafe_remove(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    const u32 index = tl_map_hash(key, map->capacity);
    TLMapEntry* entry = map->buckets[index];
    TLMapEntry* prev = NULL;

    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            if (prev == NULL) {
                map->buckets[index] = entry->next;
            } else {
                prev->next = entry->next;
            }

            TLList* value = entry->value;

            tl_string_destroy(entry->key);
            tl_memory_free(map->allocator, entry);

            map->size--;
            map->mod_count++;
            TL_PROFILER_POP_WITH(value)
        }

        prev = entry;
        entry = entry->next;
    }

    TL_PROFILER_POP_WITH(NULL)
}

u32 tl_map_unsafe_size(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    TL_PROFILER_POP_WITH(map->size)
}

u32 tl_map_unsafe_capacity(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    TL_PROFILER_POP_WITH(map->capacity)
}

b8 tl_map_unsafe_is_empty(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    TL_PROFILER_POP_WITH(map->size == 0)
}

void tl_map_unsafe_clear(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    for (u32 i = 0; i < map->capacity; i++) {
        TLMapEntry* entry = map->buckets[i];

        while (entry != NULL) {
            TLMapEntry* next = entry->next;
            tl_map_free_entry(map->allocator, entry);
            entry = next;
        }

        map->buckets[i] = NULL;
    }

    map->size = 0;
    map->mod_count++;

    TL_PROFILER_POP
}

#endif
