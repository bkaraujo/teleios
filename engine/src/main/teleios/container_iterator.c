#include "teleios/teleios.h"
#include "teleios/memory_types.inl"
#include "teleios/container_types.inl"


// ---------------------------------
// Iterator Lifecycle
// ---------------------------------

void tl_iterator_destroy(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    if (iterator == NULL) {
        TLWARN("Attempted to destroy NULL iterator")
        TL_PROFILER_POP
    }

    tl_memory_allocator_destroy(iterator->allocator);
    iterator = NULL;

    TL_PROFILER_POP
}

b8 tl_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL) {
        TLWARN("Attempted to iterate a NULL iterator")
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(iterator->current < iterator->size)
}

void* tl_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL) {
        TLWARN("Attempted to iterate a NULL iterator")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (iterator->current >= iterator->size) {
        TL_PROFILER_POP_WITH(NULL)
    }

    TL_PROFILER_POP_WITH(iterator->items[iterator->current++])
}

void tl_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL) {
        TLWARN("Attempted to rewind a NULL iterator")
        TL_PROFILER_POP
    }

    iterator->current = 0;
}

u32 tl_iterator_size(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL) {
        TLWARN("Attempted to query a NULL iterator")
        TL_PROFILER_POP_WITH(0)
    }

    TL_PROFILER_POP_WITH(iterator->size)
}