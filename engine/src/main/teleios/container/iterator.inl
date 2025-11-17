#ifndef __TELEIOS_CONTAINER_ITERATOR__
#define __TELEIOS_CONTAINER_ITERATOR__

#include "teleios/memory/types.inl"
#include "teleios/container/types.inl"
#include "teleios/teleios.h"

// ---------------------------------
// Iterator Lifecycle
// ---------------------------------

void tl_iterator_destroy(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL) {
        TLWARN("Attempted to destroy NULL iterator")
        TL_PROFILER_POP
    }

    // Free the state (allocated by container)
    if (iterator->state != NULL) {
        tl_memory_free(iterator->allocator, iterator->state);
    }

    // Free the iterator itself
    tl_memory_free(iterator->allocator, iterator);

    TL_PROFILER_POP
}

// ---------------------------------
// Iterator Navigation
// ---------------------------------

b8 tl_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    if (iterator == NULL) {
        TLWARN("Attempted to use NULL iterator")
        TL_PROFILER_POP_WITH(false)
    }

    // Check for concurrent modification before proceeding
    if (iterator->has_modified) {
        iterator->has_modified(iterator);
    }

    // Delegate to container-specific implementation
    b8 has_next = false;
    if (iterator->has_next) {
        has_next = iterator->has_next(iterator);
    }

    TL_PROFILER_POP_WITH(has_next)
}

void* tl_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    if (iterator == NULL) {
        TLWARN("Attempted to use NULL iterator")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Check for concurrent modification
    if (iterator->has_modified) {
        iterator->has_modified(iterator);
    }

    // Delegate to container-specific implementation
    void* item = NULL;
    if (iterator->next) {
        item = iterator->next(iterator);
    }

    TL_PROFILER_POP_WITH(item)
}

void tl_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    if (iterator == NULL) {
        TLWARN("Attempted to rewind NULL iterator")
        TL_PROFILER_POP
    }

    // Check for concurrent modification
    if (iterator->has_modified) {
        iterator->has_modified(iterator);
    }

    // Delegate to container-specific implementation
    if (iterator->rewind) {
        iterator->rewind(iterator);
    }

    TL_PROFILER_POP
}

void tl_iterator_resync(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    if (iterator == NULL) {
        TLWARN("Attempted to resync NULL iterator")
        TL_PROFILER_POP
    }

    // Delegate to container-specific implementation
    // This will acquire the container's mutex and update:
    // - expected_mod_count
    // - size
    // - state (rewind to beginning)
    if (iterator->resync) {
        iterator->resync(iterator);
    }

    TL_PROFILER_POP
}

u32 tl_iterator_size(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    if (iterator == NULL) {
        TLWARN("Attempted to get size of NULL iterator")
        TL_PROFILER_POP_WITH(0)
    }

    TL_PROFILER_POP_WITH(iterator->size)
}
#endif