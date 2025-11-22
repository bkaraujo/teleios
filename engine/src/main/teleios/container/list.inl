#ifndef __TELEIOS_CONTAINER_LIST__
#define __TELEIOS_CONTAINER_LIST__

#include "teleios/memory/types.inl"
#include "teleios/container/types.inl"
#include "teleios/teleios.h"
#include "teleios/container/list_safe.inl"
#include "teleios/container/list_unsafe.inl"

// ---------------------------------
// TLList Implementation
// ---------------------------------

TLList* tl_list_create(TLAllocator* allocator, const b8 thread_safe) {
    TL_PROFILER_PUSH_WITH("0x%p, %d", allocator, thread_safe)

    if (allocator == NULL) {
        TLERROR("Attempted to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLList* list = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_LIST, sizeof(TLList));
    if (list == NULL) {
        TLERROR("Failed to allocate TLList structure")
        TL_PROFILER_POP_WITH(NULL)
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->mod_count = 0;
    list->allocator = allocator;
    list->thread_safe = thread_safe;
    list->mutex = NULL;

    if (thread_safe) {
        list->mutex = tl_mutex_create(allocator);
        if (!list->mutex) {
            TLERROR("Failed to create mutex for list")
            tl_memory_free(allocator, list);
            TL_PROFILER_POP_WITH(NULL)
        }
    }

    TLTRACE("List created: thread_safe=%d", thread_safe);
    TL_PROFILER_POP_WITH(list)
}

void tl_list_destroy(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TL_PROFILER_POP
    }

    TLTRACE("Destroying list: size=%u", list->size);

    tl_list_clear(list);

    if (list->mutex) tl_mutex_destroy(list->mutex);
    tl_memory_free(list->allocator, list);

    TL_PROFILER_POP
}

// ---------------------------------
// TLList Dispatchers
// ---------------------------------

void tl_list_push_front(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    if (list->thread_safe) {
        tl_list_safe_push_front(list, data);
        TL_PROFILER_POP
    }
    tl_list_unsafe_push_front(list, data);
    TL_PROFILER_POP
}

void tl_list_push_back(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    if (list->thread_safe) {
        tl_list_safe_push_back(list, data);
        TL_PROFILER_POP
    }
    tl_list_unsafe_push_back(list, data);
    TL_PROFILER_POP
}

void tl_list_insert_after(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    if (node == NULL) {
        tl_list_push_back(list, data);
        TL_PROFILER_POP
    }

    if (list->thread_safe) {
        tl_list_safe_insert_after(list, node, data);
        TL_PROFILER_POP
    }
    tl_list_unsafe_insert_after(list, node, data);
    TL_PROFILER_POP
}

void tl_list_insert_before(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    if (node == NULL) {
        tl_list_push_front(list, data);
        TL_PROFILER_POP
    }

    if (list->thread_safe) {
        tl_list_safe_insert_before(list, node, data);
        TL_PROFILER_POP
    }
    tl_list_unsafe_insert_before(list, node, data);
    TL_PROFILER_POP
}

void* tl_list_pop_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->head == NULL) {
        TLWARN("Cannot pop from empty list")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->thread_safe) TL_PROFILER_POP_WITH(tl_list_safe_pop_front(list));
    TL_PROFILER_POP_WITH(tl_list_unsafe_pop_front(list));
}

void* tl_list_pop_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->tail == NULL) {
        TLWARN("Cannot pop from empty list")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->thread_safe) TL_PROFILER_POP_WITH(tl_list_safe_pop_back(list));
    TL_PROFILER_POP_WITH(tl_list_unsafe_pop_back(list));
}

void* tl_list_remove(TLList* list, TLListNode* node) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, node)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (node == NULL) {
        TLWARN("Attempted to remove a NULL TLListNode")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->thread_safe) TL_PROFILER_POP_WITH(tl_list_safe_remove(list, node));
    TL_PROFILER_POP_WITH(tl_list_unsafe_remove(list, node));
}

void* tl_list_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->head == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->thread_safe) TL_PROFILER_POP_WITH(tl_list_safe_front(list));
    TL_PROFILER_POP_WITH(tl_list_unsafe_front(list));
}

void* tl_list_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->tail == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }

    if (list->thread_safe) TL_PROFILER_POP_WITH(tl_list_safe_back(list));
    TL_PROFILER_POP_WITH(tl_list_unsafe_back(list));
}

u32 tl_list_size(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(0)
    }

    if (list->thread_safe) TL_PROFILER_POP_WITH(tl_list_safe_size(list));
    TL_PROFILER_POP_WITH(tl_list_unsafe_size(list));
}

b8 tl_list_is_empty(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(true)
    }

    if (list->thread_safe) TL_PROFILER_POP_WITH(tl_list_safe_is_empty(list));
    TL_PROFILER_POP_WITH(tl_list_unsafe_is_empty(list));
}

void tl_list_clear(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    if (list->thread_safe) {
        tl_list_safe_clear(list);
        TL_PROFILER_POP
    }
    tl_list_unsafe_clear(list);
    TL_PROFILER_POP
}

// ---------------------------------
// List Iterator Implementation
// ---------------------------------

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
