#include "teleios/core.h"

struct TLStack {
    TLList* list;
};

TLStack* tl_stack_create(TLMemoryArena* arena)  {
    TLSTACKPUSHA("0x%p", arena)
    TLStack* stack = tl_memory_alloc(arena, sizeof(TLStack), TL_MEMORY_CONTAINER_STACK);
    stack->list = tl_list_create(arena);
    TLSTACKPOPV(stack)
}

void tl_stack_push(TLStack* stack, void* value) {
    TLSTACKPUSHA("0x%p, 0x%p", stack, value)
    tl_list_add(stack->list, value);
    TLSTACKPOP
}

void* tl_stack_pop(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    void* value = tl_stack_peek(stack);
    tl_list_remove(stack->list, value);
    TLSTACKPOPV(value)
}

void* tl_stack_peek(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    void* value = NULL;
    TLIterator* iterator = tl_list_iterator_create(stack->list);
    void* next = tl_list_iterator_next(iterator);
    while (next != NULL) { value = next; next = tl_list_iterator_next(iterator); }
    TLSTACKPOPV(value)
}

u64 tl_stack_length(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    TLSTACKPOPV(tl_list_length(stack->list))
}