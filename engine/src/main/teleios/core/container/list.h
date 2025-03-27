#ifndef __TELEIOS_CORE_CONTAINER_LIST__
#define __TELEIOS_CORE_CONTAINER_LIST__

#include "teleios/defines.h"
#include "teleios/core/types.h"

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
void* tl_list_iterator_next(TLIterator* iterator);

#endif //__TELEIOS_CORE_CONTAINER_LIST__
