#ifndef __TELEIOS_CONTAINER__
#define __TELEIOS_CONTAINER__

#include "teleios/defines.h"

TLList* tl_list_create(TLMemoryArena* arena);
void* tl_list_search(TLList* list, b8 (PFN_filter)(void *value));

void tl_list_add(TLList* list, void *value);
void tl_list_remove(TLList* list, void *value);
b8 tl_list_after(TLList* list, void *item, void *value);
b8 tl_list_before(TLList* list, void *item, void *value);

b8 lt_list_contains(TLList* list, void *value);

#endif // __TELEIOS_CONTAINER__