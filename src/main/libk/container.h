// #ifndef __LIBK_CONTAINER__
// #define __LIBK_CONTAINER__
//
// #include "libk/defines.h"
//
// typedef struct KIterator KIterator;
//
// void* k_iterator_next(KIterator* iterator);
//
// typedef struct KList KList;
//
// KList* k_list_create(KAllocator *allocator);
// void* k_list_search(KList* list, b8 (*PFN_filter)(void *value));
// void k_list_foreach(KList* list, void (*PFN_handler)(void *value));
// u64 k_list_length(KList* list);
// void k_list_add(KList* list, void *value);
// void k_list_remove(KList* list, void *value);
// b8 k_list_after(KList* list, void *item, void *value);
// b8 k_list_before(KList* list, void *item, void *value);
// b8 k_list_contains(KList* list, void *value);
// KIterator* k_list_iterator_create(KAllocator *allocator, KList* list);
//
// typedef KList KStack;
//
// KStack* k_stack_create(KAllocator *allocator);
// void k_stack_push(KStack* stack, void* value);
// void* k_stack_peek(KStack* stack);
// void* k_stack_pop(KStack* stack);
// u64 k_stack_length(KStack* stack);
// KIterator* k_stack_iterator_create(KStack* stack);
//
// typedef struct KMap KMap;
//
// KMap* k_map_create(KAllocator *allocator);
// void k_map_put(KMap* map, const char *key, void *value);
// void* k_map_get(KMap* map, const char *key);
// void k_map_remove(KMap* map, const char *key);
// b8 k_map_contains(KMap* map, const char *key);
// u16 k_map_length(KMap* map);
// KIterator* k_map_keys(KAllocator *allocator, KMap* map);
//
// #endif