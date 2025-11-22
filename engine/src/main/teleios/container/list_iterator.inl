#ifndef __TELEIOS_CONTAINER_LIST_ITERATOR__
#define __TELEIOS_CONTAINER_LIST_ITERATOR__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

typedef struct {
    TLListNode* current_node;
} TLListIteratorState;

static void tl_list_iterator_check_modification(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLList* list = (const TLList*)iterator->source;

    if (list->thread_safe) tl_mutex_lock(list->mutex);
    const u32 current_mod_count = list->mod_count;
    if (list->thread_safe) tl_mutex_unlock(list->mutex);

    if (current_mod_count != iterator->expected_mod_count) {
        TLFATAL("Concurrent modification detected during list iteration (expected=%u, actual=%u)",
                iterator->expected_mod_count, current_mod_count)
    }

    TL_PROFILER_POP
}

static b8 tl_list_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLListIteratorState* state = (const TLListIteratorState*)iterator->state;

    TL_PROFILER_POP_WITH(state->current_node != NULL)
}

static void* tl_list_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLListIteratorState* state = (TLListIteratorState*)iterator->state;

    if (state->current_node == NULL) {
        TLWARN("Iterator exhausted")
        TL_PROFILER_POP_WITH(NULL)
    }

    void* data = state->current_node->data;
    state->current_node = state->current_node->next;

    TL_PROFILER_POP_WITH(data)
}

static void tl_list_iterator_rewind(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    const TLList* list = (const TLList*)iterator->source;
    TLListIteratorState* state = (TLListIteratorState*)iterator->state;

    if (list->thread_safe) tl_mutex_lock(list->mutex);
    state->current_node = list->head;
    if (list->thread_safe) tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

static void tl_list_iterator_resync(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)

    TLList* list = (TLList*)iterator->source;
    TLListIteratorState* state = (TLListIteratorState*)iterator->state;

    if (list->thread_safe) tl_mutex_lock(list->mutex);

    iterator->expected_mod_count = list->mod_count;
    iterator->size = list->size;
    state->current_node = list->head;

    if (list->thread_safe) tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

TLIterator* tl_list_iterator(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->thread_safe) tl_mutex_lock(list->mutex);

    TLIterator* iterator = tl_memory_alloc(list->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));
    TLListIteratorState* state = tl_memory_alloc(list->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLListIteratorState));

    state->current_node = list->head;

    iterator->source = list;
    iterator->expected_mod_count = list->mod_count;
    iterator->size = list->size;
    iterator->state = state;
    iterator->allocator = list->allocator;

    iterator->has_modified = tl_list_iterator_check_modification;
    iterator->has_next = tl_list_iterator_has_next;
    iterator->next = tl_list_iterator_next;
    iterator->rewind = tl_list_iterator_rewind;
    iterator->resync = tl_list_iterator_resync;

    if (list->thread_safe) tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(iterator)
}


#endif
