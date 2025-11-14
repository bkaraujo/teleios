#include "teleios/teleios.h"
#include "teleios/memory_types.inl"
#include "teleios/container_types.inl"

#define ENSURE_OR(r) {                          \
    if (iterator == NULL) {                     \
        TLWARN("Attempted use NULL iterator")   \
        TL_PROFILER_POP_WITH(r)                 \
    }                                           \
}

#define ENSURE_OR_NULL {                        \
    if (iterator == NULL) {                     \
        TLWARN("Attempted use NULL iterator")   \
        TL_PROFILER_POP                         \
    }                                           \
}

void tl_iterator_destroy(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    ENSURE_OR_NULL
    tl_memory_allocator_destroy(iterator->allocator);
    iterator = NULL;

    TL_PROFILER_POP
}

b8 tl_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    ENSURE_OR( false)
    TL_PROFILER_POP_WITH(iterator->current < iterator->size)
}

void* tl_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    ENSURE_OR(NULL)
    if (iterator->current >= iterator->size) TL_PROFILER_POP_WITH(NULL)
    TL_PROFILER_POP_WITH(iterator->items[iterator->current++])
}

void tl_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    ENSURE_OR_NULL
    iterator->current = 0;
}

u32 tl_iterator_size(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    ENSURE_OR(0)
    TL_PROFILER_POP_WITH(iterator->size)
}