#include "teleios/teleios.h"
#include "teleios/memory_types.inl"
#include "teleios/container_types.inl"
#include "teleios/strings.h"

// ---------------------------------
// Internal Helper Functions
// ---------------------------------

/**
 * @brief Compute hash of a TLString
 *
 * Uses the FNV-1a hash algorithm which is fast and has good distribution
 * for strings.
 */
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

/**
 * @brief Create a new map entry
 */
static TLMapEntry* tl_map_create_entry(TLAllocator* allocator, const TLString* key, TLList* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", allocator, key, value)

    TLMapEntry* entry = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_MAP, sizeof(TLMapEntry));
    entry->key = tl_string_copy(key);
    entry->value = value;

    TL_PROFILER_POP_WITH(entry)
}

/**
 * @brief Free a map entry and its key/value
 */
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
// Map Lifecycle
// ---------------------------------

TLMap* tl_map_create(TLAllocator* allocator, const u32 capacity) {
    TL_PROFILER_PUSH_WITH("0x%p, %u", allocator, capacity)

    if (allocator == NULL) TLFATAL("allocator is NULL")

    TLMap* map = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_MAP, sizeof(TLMap));
    map->buckets = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_MAP, sizeof(TLMapEntry*) * capacity);
    map->capacity = tl_number_next_power_of_2(capacity == 0 ? 16 : capacity);
    map->load_factor = 0.75f;
    map->allocator = allocator;
    map->mutex = tl_mutex_create(allocator);

    TL_PROFILER_POP_WITH(map)
}

void tl_map_destroy(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLWARN("Attempted to destroy NULL map")
        TL_PROFILER_POP
    }

    // Clear all entries first
    tl_map_clear(map);

    // Destroy mutex
    if (map->mutex) tl_mutex_destroy(map->mutex);

    // Free buckets array and map structure
    tl_memory_free(map->allocator, map->buckets);
    tl_memory_free(map->allocator, map);

    TL_PROFILER_POP
}

// ---------------------------------
// Map Operations
// ---------------------------------

TLList* tl_map_get(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }
    if (key == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(map->mutex);

    const u32 index = tl_map_hash(key, map->capacity);
    const TLMapEntry* entry = map->buckets[index];

    // Linear search in bucket chain
    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            TLList* value = entry->value;
            tl_mutex_unlock(map->mutex);
            TL_PROFILER_POP_WITH(value)
        }
        entry = entry->next;
    }

    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(NULL)
}

TLList* tl_map_get_or_create(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) TLFATAL("map is NULL")
    if (key == NULL) TLFATAL("key is NULL")

    tl_mutex_lock(map->mutex);

    const u32 index = tl_map_hash(key, map->capacity);
    TLMapEntry* entry = map->buckets[index];

    // Search for existing entry
    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            TLList* value = entry->value;
            tl_mutex_unlock(map->mutex);
            TL_PROFILER_POP_WITH(value)
        }
        entry = entry->next;
    }

    // Key doesn't exist - create new entry
    TLList* new_list = tl_list_create(map->allocator);
    TLMapEntry* new_entry = tl_map_create_entry(map->allocator, key, new_list);

    if (new_entry == NULL) {
        tl_list_destroy(new_list);
        tl_mutex_unlock(map->mutex);
        TLERROR("Failed to create map entry")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Insert at head of bucket chain
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->size++;

    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(new_list)
}

void tl_map_put(TLMap* map, TLString* key, void* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", map, key, value)

    if (map == NULL) TLFATAL("map is NULL")
    if (key == NULL) TLFATAL("key is NULL")

    // Get or create list for this key
    TLList* list = tl_map_get_or_create(map, key);

    // Add value to the list
    if (list != NULL) {
        tl_list_push_back(list, value);
    }

    // Clean up temporary key
    tl_string_destroy(key);

    TL_PROFILER_POP
}

b8 tl_map_contains(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) {
        TL_PROFILER_POP_WITH(false)
    }
    if (key == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    tl_mutex_lock(map->mutex);

    const u32 index = tl_map_hash(key, map->capacity);
    TLMapEntry* entry = map->buckets[index];

    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            tl_mutex_unlock(map->mutex);
            TL_PROFILER_POP_WITH(true)
        }
        entry = entry->next;
    }

    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(false)
}

TLList* tl_map_remove(TLMap* map, const TLString* key) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", map, key)

    if (map == NULL) TLFATAL("map is NULL")
    if (key == NULL) TLFATAL("key is NULL")

    tl_mutex_lock(map->mutex);

    const u32 index = tl_map_hash(key, map->capacity);
    TLMapEntry* entry = map->buckets[index];
    TLMapEntry* prev = NULL;

    while (entry != NULL) {
        if (tl_string_equals(entry->key, key)) {
            // Found the entry - remove it from chain
            if (prev == NULL) {
                // First in chain
                map->buckets[index] = entry->next;
            } else {
                // Middle or end of chain
                prev->next = entry->next;
            }

            TLList* value = entry->value;

            // Free entry but not its value (caller takes ownership)
            tl_string_destroy(entry->key);
            tl_memory_free(map->allocator, entry);

            map->size--;
            tl_mutex_unlock(map->mutex);
            TL_PROFILER_POP_WITH(value)
        }

        prev = entry;
        entry = entry->next;
    }

    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(NULL)
}

// ---------------------------------
// Map Queries
// ---------------------------------

u32 tl_map_size(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TL_PROFILER_POP_WITH(0)
    }

    tl_mutex_lock(map->mutex);
    const u32 size = map->size;
    tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP_WITH(size)
}

u32 tl_map_capacity(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TL_PROFILER_POP_WITH(0)
    }

    tl_mutex_lock(map->mutex);
    const u32 capacity = map->capacity;
    tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP_WITH(capacity)
}

b8 tl_map_is_empty(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TL_PROFILER_POP_WITH(true)
    }

    tl_mutex_lock(map->mutex);
    const b8 empty = (map->size == 0);
    tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP_WITH(empty)
}

void tl_map_clear(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLWARN("Attempted to clear NULL map")
        TL_PROFILER_POP
    }

    tl_mutex_lock(map->mutex);

    // Iterate all buckets
    for (u32 i = 0; i < map->capacity; i++) {
        TLMapEntry* entry = map->buckets[i];

        // Free all entries in this bucket chain
        while (entry != NULL) {
            TLMapEntry* next = entry->next;
            tl_map_free_entry(map->allocator, entry);
            entry = next;
        }

        map->buckets[i] = NULL;
    }

    map->size = 0;
    tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP
}

TLIterator* tl_map_keys(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)

    if (map == NULL) {
        TLWARN("Attempt to iterate over a NULL map")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(map->mutex);

    if (map->size == 0) {
        tl_mutex_unlock(map->mutex);
        TL_PROFILER_POP_WITH(NULL)
    }


    // Lock list to create thread-safe snapshot
    tl_mutex_lock(map->mutex);

    TLAllocator* allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    TLIterator* iterator = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));
    iterator->allocator = allocator;
    iterator->current = 0;

    iterator->size = map->size;

    // Empty list - create empty iterator
    if (iterator->size == 0) {
        tl_mutex_unlock(map->mutex);
        iterator->items = NULL;

        TL_PROFILER_POP_WITH(iterator)
    }

    // Allocate iterator structure
    iterator->items = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(void*) * iterator->size);

    // Copy all data pointers into contiguous array
    // This is the only part that needs the lock - after this, iteration is lock-free
    TLMapEntry** node = map->buckets;

    u32 index = 0;
    while (node != NULL && index < iterator->size) {
        iterator->items[index] = node[index]->key;
        index++;
    }

    // Unlock - snapshot is complete
    tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

