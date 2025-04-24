#include "libk/libk.h"

struct KCollection {
    u64 *key;
    u64 *value;
    KAllocator *allocator;
    KCollectionType type;
    u16 length;
    u16 size;
};

KCollection* k_collection_create(KAllocator *allocator, const KCollectionType type) {
    K_FRAME_PUSH_WITH("0x%p, %d", allocator, type)

    if (allocator == NULL) { KWARN("KAllocator is NULL"); K_FRAME_POP_WITH(NULL) }
    KCollection *collection = k_memory_allocator_alloc(allocator, sizeof(KCollection), K_MEMORY_COLLECTION);
    collection->allocator = allocator;
    collection->type = type;
    collection->size = 10;
    collection->key = k_memory_allocator_alloc(collection->allocator, collection->size * K_SIZE_POINTER, K_MEMORY_COLLECTION_KEY);
    collection->value = k_memory_allocator_alloc(collection->allocator, collection->size * K_SIZE_POINTER, K_MEMORY_COLLECTION_VALUE);

    K_FRAME_POP_WITH(collection)
}

KAllocator* k_collection_get_allocator(KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p", collection)
    KAllocator *allocator = collection->allocator;
    K_FRAME_POP_WITH(allocator)
}

b8 k_collection_is_full(KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p", collection)
    const b8 is_full = collection->length == collection->size;
    K_FRAME_POP_WITH(is_full)
}

b8 k_collection_is_empty(KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p", collection)
    const b8 is_empty = collection->length == 0;
    K_FRAME_POP_WITH(is_empty)
}

u16 k_collection_length(KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p", collection)
    const u16 length = collection->length;
    K_FRAME_POP_WITH(length)
}

void k_collection_clear(KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p", collection)
    k_memory_set(collection->key  , 0, collection->size * K_SIZE_POINTER);
    k_memory_set(collection->value, 0, collection->size * K_SIZE_POINTER);
    collection->length = 0;
    K_FRAME_POP
}

void k_collection_destroy(KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p", collection)

    if (collection == NULL) { KWARN("KCollection is NULL"); K_FRAME_POP }
    if (collection->key != NULL) k_memory_allocator_free(collection->allocator, collection->key);
    if (collection->value != NULL) k_memory_allocator_free(collection->allocator, collection->value);
    k_memory_allocator_free(collection->allocator, collection);

    K_FRAME_POP
}

static void k_collection_resize(KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p", collection)

    const u16 new_size = (u16)(((f32)collection->size * 1.75f) + 1);

    void *new_key = k_memory_allocator_alloc(collection->allocator, new_size * K_SIZE_POINTER, K_MEMORY_COLLECTION_KEY);
    k_memory_copy(new_key, collection->key, collection->size);
    k_memory_allocator_free(collection->allocator, collection->key);
    collection->key = new_key;

    void *new_value = k_memory_allocator_alloc(collection->allocator, new_size * K_SIZE_POINTER, K_MEMORY_COLLECTION_VALUE);
    k_memory_copy(new_value, collection->value, collection->size);
    k_memory_allocator_free(collection->allocator, collection->value);
    collection->value = new_value;

    K_FRAME_POP
}

static void k_collection_push_value(KCollection *collection, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", collection, value)

    if (value == NULL) { KWARN("value is NULL"); K_FRAME_POP }
    if (k_collection_is_full(collection)) k_collection_resize(collection);

    k_memory_copy(collection->key + collection->length, (void*) collection->length, K_SIZE_POINTER);
    k_memory_copy(collection->value + collection->length, value, K_SIZE_POINTER);
    collection->length++;

    K_FRAME_POP
}

static void k_collection_push_key(KCollection *collection, void *key, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, 0x%p", collection, key, value)

    if (key == NULL) { KWARN("key is NULL"); K_FRAME_POP }
    if (value == NULL) { KWARN("value is NULL"); K_FRAME_POP }
    if (k_collection_is_full(collection)) k_collection_resize(collection);

    k_memory_copy(collection->key   + collection->length, key  , K_SIZE_POINTER);
    k_memory_copy(collection->value + collection->length, value, K_SIZE_POINTER);
    collection->length++;

    K_FRAME_POP
}

void k_collection_add(KCollection *collection, void *key, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, 0x%p", collection, key, value)

    if (collection == NULL) { KWARN("KCollection is NULL"); K_FRAME_POP }
    switch (collection->type) {
        case K_COLLECTION_LIST:
        case K_COLLECTION_STACK: k_collection_push_value(collection     , value); break;
        case K_COLLECTION_MAP  : k_collection_push_key  (collection, key, value); break;
    }

    K_FRAME_POP
}

static void* k_collection_get_index(KCollection *collection, void *key) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", collection, key)

    const u16 index = (u16) key;
    if (index > collection->length) {
        KERROR("Index [%d] beyond range[%d]", index, collection->length);
        K_FRAME_POP_WITH(NULL)
    }

    void *value = collection->value + index;
    K_FRAME_POP_WITH(value)
}

static void* k_collection_get_key(KCollection *collection, KString *key) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", collection, key)

    void *value = NULL;
    for (u16 i = 0; i < collection->length; ++i) {
        if (!k_string_equals((KString*)(collection->key + i), k_string(key))) continue;

        value = collection->value + i;
        break;
    }

    K_FRAME_POP_WITH(value)
}

void* k_collection_get(KCollection *collection, void *key) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", collection, key)
    if (collection == NULL) { KWARN("KCollection is NULL"); K_FRAME_POP_WITH(NULL) }
    if (key == NULL) { KWARN("key is NULL"); K_FRAME_POP_WITH(NULL) }

    void *value = NULL;
    switch (collection->type) {
        case K_COLLECTION_LIST:
        case K_COLLECTION_STACK: value = k_collection_get_index(collection, key); break;
        case K_COLLECTION_MAP  : value = k_collection_get_key  (collection, key); break;
    }

    K_FRAME_POP_WITH(value)
}

static void k_collection_rem_at(KCollection *collection, u16 index) {
    K_FRAME_PUSH_WITH("0x%p, %d", collection, index)
    if (index > collection->length) {
        KERROR("Index [%d] beyond range[%d]", index, collection->length);
        K_FRAME_POP
    }

    k_memory_copy(collection->key + index - 1, collection->key + index, collection->length - index);
    k_memory_copy(collection->value + index - 1, collection->value + index, collection->length - index);
    collection->length--;

    k_memory_set(collection->key + collection->length, 0, (collection->size - collection->length) * K_SIZE_POINTER);
    k_memory_set(collection->value + collection->length, 0, (collection->size - collection->length) * K_SIZE_POINTER);

    K_FRAME_POP
}

static void k_collection_rem_index(KCollection *collection, void *key) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", collection, key)
    k_collection_rem_at(collection, (u16) key);
    K_FRAME_POP
}

static void k_collection_rem_key(KCollection *collection, void *key) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", collection, key)
    for (u16 i = 0; i < collection->length; ++i) {
        if (!k_string_equals((KString*)(collection->key + i), k_string(key))) continue;
        k_collection_rem_at(collection, i);
        break;
    }
    K_FRAME_POP
}

void k_collection_rem(KCollection *collection, void *key) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", collection, key)
    switch (collection->type) {
        case K_COLLECTION_LIST:
        case K_COLLECTION_STACK: k_collection_rem_index(collection, key); break;
        case K_COLLECTION_MAP  : k_collection_rem_key  (collection, key); break;
    }
    K_FRAME_POP
}

// ########################################################################
//
//                                 ITERATOR
//
// ########################################################################

struct KIterator {u64 capacity; u64 length; u64* node; void* (*next)(KIterator*); };

static void* k_collection_iterator_next(KIterator *iterator) {
    K_FRAME_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL ) { KERROR("KIterator is NULL") K_FRAME_POP_WITH(NULL)}
    if (iterator->node == NULL) { KWARN("KIterator exhausted") K_FRAME_POP_WITH(NULL) }

    void *value = iterator->node + iterator->length;
    iterator->length++;

    if (iterator->length == iterator->capacity) {
        iterator->node = NULL;
        iterator->length = 0;
        iterator->capacity = 0;
    }

    K_FRAME_POP_WITH(value)
}

void* k_iterator_create(KAllocator *allocator, KCollection *collection) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", allocator, collection)
    KIterator* iterator = k_memory_allocator_alloc(allocator == NULL ? collection->allocator : allocator, sizeof(KIterator), K_MEMORY_CONTAINER_ITERATOR);
    iterator->length = 0;
    iterator->capacity = collection->length;
    iterator->next = k_collection_iterator_next;
    iterator->node = k_memory_allocator_alloc(allocator, collection->length * K_SIZE_POINTER, K_MEMORY_COLLECTION_ITERATOR);
    switch (collection->type) {
        case K_COLLECTION_LIST:
        case K_COLLECTION_STACK: k_memory_copy(iterator->node, collection->value, collection->length * K_SIZE_POINTER); break;
        case K_COLLECTION_MAP  : k_memory_copy(iterator->node, collection->key, collection->length * K_SIZE_POINTER); break;
    }

    K_FRAME_POP_WITH(iterator)
}

b8 k_iterator_has_next(KIterator* iterator) {
    K_FRAME_PUSH_WITH("0x%p", iterator)
    const b8 has_next = iterator->node != NULL;
    K_FRAME_POP_WITH(has_next)
}

void* k_iterator_next(KIterator* iterator) {
    K_FRAME_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        K_FRAME_POP_WITH(NULL)
    }

    void *next = iterator->next(iterator);
    K_FRAME_POP_WITH(next)
}