#include "teleios/core/meta.h"
#include "teleios/core/memory.h"
#include "teleios/core/logger.h"
#include "teleios/core/container/types.h"
#include "teleios/core/container/list.h"
#include "teleios/core/container/stack.h"

TLStack* tl_stack_create(TLMemoryArena *arena)  {
    TLSTACKPUSHA("0x%p", arena)
    TLStack* stack = tl_memory_alloc(arena, sizeof(TLStack), TL_MEMORY_CONTAINER_STACK);
    stack = tl_list_create(arena);
    TLSTACKPOPV(stack)
}

void tl_stack_push(TLStack* stack, void* value) {
    TLSTACKPUSHA("0x%p, 0x%p", stack, value)
    tl_list_add(stack, value);
    TLSTACKPOP
}

void* tl_stack_peek(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    void* value = NULL;
    TLIterator* iterator = tl_list_iterator_create(stack);
    void* next = tl_list_iterator_next(iterator);
    while (next != NULL) { value = next; next = tl_list_iterator_next(iterator); }
    TLSTACKPOPV(value)
}

void* tl_stack_pop(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    void* value = tl_stack_peek(stack);
    tl_list_remove(stack, value);
    TLSTACKPOPV(value)
}

u64 tl_stack_length(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    TLSTACKPOPV(tl_list_length(stack))
}