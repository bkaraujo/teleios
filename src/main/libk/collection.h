#ifndef __LIBK_COLLECTION__
#define __LIBK_COLLECTION__

#include "libk/defines.h"

typedef enum {
    K_COLLECTION_STACK,
    K_COLLECTION_LIST,
    K_COLLECTION_MAP,
} KCollectionType;

KCollection* k_collection_create(KAllocator *allocator, KCollectionType type);
KAllocator* k_collection_get_allocator(KCollection *collection);
void k_collection_destroy(KCollection *collection);
void k_collection_add(KCollection *collection, void *key, void *value);
void* k_collection_get(KCollection *collection, void *key);
void k_collection_rem(KCollection *collection, void *key);
void k_collection_clear(KCollection *collection);
b8 k_collection_is_full(KCollection *collection);
b8 k_collection_is_empty(KCollection *collection);
u16 k_collection_length(KCollection *collection);


// ########################################################################
//
//                                 ITERATOR
//
// ########################################################################


void* k_iterator_create(KAllocator *allocator, KCollection *collection);
b8 k_iterator_has_next(KIterator* iterator);
void* k_iterator_next(KIterator* iterator);


// ########################################################################
//
//                                 STACK
//
// ########################################################################


#define k_stack_create(allocator) k_collection_create(allocator, K_COLLECTION_STACK)
#define k_stack_destroy(collection) k_collection_destroy(collection)
#define k_stack_is_empty(collection) k_collection_is_empty(collection)
#define k_stack_is_full(collection) k_collection_is_full(collection)
#define k_stack_push(collection, item) k_collection_add(collection, NULL, item)
#define k_stack_pop(collection) k_collection_rem(collection, k_collection_length(collection) - 1)
#define k_stack_peek(collection) k_collection_get(collection, k_collection_length(collection) - 1)


// ########################################################################
//
//                                     LIST
//
// ########################################################################


#define k_list_create(allocator) k_collection_create(allocator, K_COLLECTION_LIST)
#define k_list_destroy(collection) k_collection_destroy(collection)
#define k_list_is_empty(collection) k_collection_is_empty(collection)
#define k_list_is_full(collection) k_collection_is_full(collection)
#define k_list_add(collection, item) k_collection_add(collection, NULL, item)
#define lk_ist_delete(collection, index) k_collection_rem(collection, index)
#define k_list_get(collection, index) k_collection_get(collection, index)


// ########################################################################
//
//                                     MAP
//
// ########################################################################


#define k_map_create(allocator) k_collection_create(allocator, K_COLLECTION_MAP)
#define k_map_destroy(collection) k_collection_destroy(collection)
#define k_map_is_empty(collection) k_collection_is_empty(collection)
#define k_map_is_full(collection) k_collection_is_full(collection)
#define k_map_put(collection, key, value) k_collection_add(collection, key, value)
#define k_map_delete(collection, key) k_collection_rem(collection, key)
#define k_map_get(collection, key) k_collection_get(collection, key)

#endif