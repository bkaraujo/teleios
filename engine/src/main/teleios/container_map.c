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
    if (allocator == NULL) {
        TLERROR("Attempt to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

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
        TLERROR("Attempt to use a NULL TLMap")
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
        TLERROR("Attempt to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLERROR("Attempt to use a NULL TLString")
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
    if (map == NULL) {
        TLERROR("Attempt to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLERROR("Attempt to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

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
    map->mod_count++;

    tl_mutex_unlock(map->mutex);
    TL_PROFILER_POP_WITH(new_list)
}

void tl_map_put(TLMap* map, TLString* key, void* value) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", map, key, value)
    if (map == NULL) {
        TLERROR("Attempt to use a NULL TLMap")
        TL_PROFILER_POP
    }

    if (key == NULL) {
        TLERROR("Attempt to use a NULL TLString")
        TL_PROFILER_POP
    }

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
        TLERROR("Attempt to use a NULL TLMap")
        TL_PROFILER_POP_WITH(false)
    }

    if (key == NULL) {
        TLERROR("Attempt to use a NULL TLString")
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
    if (map == NULL) {
        TLERROR("Attempt to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (key == NULL) {
        TLERROR("Attempt to use a NULL TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

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
            map->mod_count++;
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
        TLERROR("Attempt to use a NULL TLMap")
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
        TLERROR("Attempt to use a NULL TLMap")
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
        TLERROR("Attempt to use a NULL TLMap")
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
        TLERROR("Attempt to use a NULL TLMap")
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
    map->mod_count++;
    tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP
}

// ---------------------------------
// Map Iterator Implementation
// ---------------------------------
typedef struct {
    u32 bucket_index;          // Current bucket index
    TLMapEntry* current_entry; // Current entry in bucket chain
} TLMapIteratorState;

static void tl_map_iterator_check_modification(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    const TLMap* map = (const TLMap*)iterator->source;
    if (map->mod_count != iterator->expected_mod_count) {
        TLFATAL("Concurrent modification detected during map iteration! Map was modified while being iterated (expected mod_count=%u, actual=%u)",
                iterator->expected_mod_count, map->mod_count)
    }
    TL_PROFILER_POP
}

static b8 tl_map_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    const TLMapIteratorState* state = (const TLMapIteratorState*)iterator->state;
    const b8 has_next = (state->current_entry != NULL);
    TL_PROFILER_POP_WITH(has_next)
}

static void* tl_map_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    TLMapIteratorState* state = (TLMapIteratorState*)iterator->state;
    if (state->current_entry == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }

    void* key = (void*)state->current_entry->key;

    // Move to next entry in chain, or next non-empty bucket
    state->current_entry = state->current_entry->next;

    if (state->current_entry == NULL) {
        // Current bucket chain exhausted, find next non-empty bucket
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
    TLMap* map = (TLMap*)iterator->source;
    TLMapIteratorState* state = (TLMapIteratorState*)iterator->state;

    // Find first non-empty bucket
    state->bucket_index = 0;
    state->current_entry = NULL;

    for (u32 i = 0; i < map->capacity; ++i) {
        if (map->buckets[i] != NULL) {
            state->bucket_index = i;
            state->current_entry = map->buckets[i];
            break;
        }
    }

    TL_PROFILER_POP
}

TLIterator* tl_map_keys(TLMap* map) {
    TL_PROFILER_PUSH_WITH("0x%p", map)
    if (map == NULL) {
        TLERROR("Attempt to use a NULL TLMap")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Lock map to capture current state
    tl_mutex_lock(map->mutex);

    // Allocate iterator on map's allocator
    TLIterator* iterator = tl_memory_alloc(map->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));

    // Allocate state on map's allocator
    TLMapIteratorState* state = tl_memory_alloc(map->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLMapIteratorState));

    // Find first non-empty bucket
    state->bucket_index = 0;
    state->current_entry = NULL;

    for (u32 i = 0; i < map->capacity; ++i) {
        if (map->buckets[i] != NULL) {
            state->bucket_index = i;
            state->current_entry = map->buckets[i];
            break;
        }
    }

    // Initialize fail-fast iterator with data
    iterator->source = map;
    iterator->expected_mod_count = map->mod_count;
    iterator->size = map->size;
    iterator->state = state;
    iterator->allocator = map->allocator;

    // Assign function pointers
    iterator->has_modified = tl_map_iterator_check_modification;
    iterator->has_next = tl_map_iterator_has_next;
    iterator->next = tl_map_iterator_next;
    iterator->rewind = tl_map_iterator_rewind;

    tl_mutex_unlock(map->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

