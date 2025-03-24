#ifndef __TELEIOS_CONTAINER__
#define __TELEIOS_CONTAINER__

#include "teleios/defines.h"

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

TLStack* tl_stack_create(TLMemoryArena *arena);
void tl_stack_push(TLStack* stack, void* value);
void* tl_stack_pop(TLStack* stack);
void* tl_stack_peek(TLStack* stack);
u64 tl_stack_length(TLStack* stack);

//TODO create queue api

#endif // __TELEIOS_CONTAINER__