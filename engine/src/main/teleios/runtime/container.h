#ifndef __TELEIOS_RUNTIME_CONTAINER__
#define __TELEIOS_RUNTIME_CONTAINER__

#include "teleios/defines.h"

void* tl_iterator_next(TLIterator* iterator);

TLList* tl_list_create(TLMemoryArena *arena);
void* tl_list_search(TLList* list, b8 (*PFN_filter)(void *value));
void tl_list_foreach(TLList* list, void (*PFN_handler)(void *value));
u64 tl_list_length(TLList* list);
void tl_list_add(TLList* list, void *value);
void tl_list_remove(TLList* list, void *value);
b8 tl_list_after(TLList* list, void *item, void *value);
b8 tl_list_before(TLList* list, void *item, void *value);
b8 tl_list_contains(TLList* list, void *value);
TLIterator* tl_list_iterator_create(TLList* list);

TLStack* BKS_STACK_create(TLMemoryArena *arena);
void BKS_STACK_push(TLStack* stack, void* value);
void* BKS_STACK_peek(TLStack* stack);
void* BKS_STACK_pop(TLStack* stack);
u64 BKS_STACK_length(TLStack* stack);
TLIterator* BKS_STACK_iterator_create(TLStack* stack);

TLMap* tl_map_create(TLMemoryArena *arena);
void tl_map_put(TLMap* map, const char *key, void *value);
void* tl_map_get(TLMap* map, const char *key);
void tl_map_remove(TLMap* map, const char *key);
b8 tl_map_contains(TLMap* map, const char *key);
u16 tl_map_length(TLMap* map);
TLIterator* tl_map_keys(TLMap* map);


#endif // __TELEIOS_RUNTIME_CONTAINER__