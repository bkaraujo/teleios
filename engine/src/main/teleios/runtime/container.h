#ifndef __TELEIOS_RUNTIME_CONTAINER__
#define __TELEIOS_RUNTIME_CONTAINER__

#include "teleios/defines.h"

void* tl_iterator_next(TLIterator* iterator);

TLList* tl_list_create(KAllocator *allocator);
void* tl_list_search(TLList* list, b8 (*PFN_filter)(void *value));
void tl_list_foreach(TLList* list, void (*PFN_handler)(void *value));
u64 tl_list_length(TLList* list);
void tl_list_add(TLList* list, void *value);
void tl_list_remove(TLList* list, void *value);
b8 tl_list_after(TLList* list, void *item, void *value);
b8 tl_list_before(TLList* list, void *item, void *value);
b8 tl_list_contains(TLList* list, void *value);
TLIterator* tl_list_iterator_create(KAllocator *allocator, TLList* list);

TLStack* tl_stack_create(KAllocator *allocator);
void tl_stack_push(TLStack* stack, void* value);
void* tl_stack_peek(TLStack* stack);
void* tl_stack_pop(TLStack* stack);
u64 tl_stack_length(TLStack* stack);
TLIterator* tl_stack_iterator_create(TLStack* stack);

TLMap* tl_map_create(KAllocator *allocator);
void tl_map_put(TLMap* map, const char *key, void *value);
void* tl_map_get(TLMap* map, const char *key);
void tl_map_remove(TLMap* map, const char *key);
b8 tl_map_contains(TLMap* map, const char *key);
u16 tl_map_length(TLMap* map);
TLIterator* tl_map_keys(KAllocator *allocator, TLMap* map);


#endif // __TELEIOS_RUNTIME_CONTAINER__