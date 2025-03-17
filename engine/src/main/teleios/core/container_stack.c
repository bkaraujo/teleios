#include "teleios/core.h"

struct TLStack {
    TLList* list;
};

TLStack* tl_stack_create(TLMemoryArena* arena)  {
    TLTRACE(">> tl_stack_create(0x%p)", arena)
    TLList* list = tl_list_create(arena);
    TLStack* stack = tl_memory_alloc(arena, sizeof(TLStack), TL_MEMORY_CONTAINER_STACK);
    TLTRACE("<< tl_stack_create(0x%p)", arena)
    return stack;
}

void tl_stack_push(TLStack* stack, void* value) {
    TLTRACE(">> tl_stack_push(0x%p, 0x%p)", stack, value)
    tl_list_add(stack->list, value);
    TLTRACE("<< tl_stack_push(0x%p, 0x%p)", stack, value)
}

void* tl_stack_pop(TLStack* stack) {
    TLTRACE(">> tl_stack_pop(0x%p)", stack)
    void* value = tl_stack_peek(stack);
    tl_list_remove(stack->list, value);
    TLTRACE("<< tl_stack_pop(0x%p)", stack)
    return value;
}

void* tl_stack_peek(TLStack* stack) {
    TLTRACE(">> tl_stack_peek(0x%p)", stack)
    void* value = NULL;
    TLIterator* iterator = tl_list_iterator_create(stack->list);
    void* next = tl_list_iterator_next(iterator);
    while (next != NULL) { value = next; next = tl_list_iterator_next(iterator); }
    TLTRACE("<< tl_stack_peek(0x%p)", stack)
    return value;
}

u64 tl_stack_length(TLStack* stack) {
    TLTRACE(">> tl_stack_length(0x%p)", stack)
    u64 length = tl_list_length(stack->list);
    TLTRACE("<< tl_stack_length(0x%p)", stack)
    return length;
}