#ifndef __TELEIOS_CONTAINER_LIST__
#define __TELEIOS_CONTAINER_LIST__

#include "teleios/memory/types.inl"
#include "teleios/container/types.inl"
#include "teleios/teleios.h"

// ---------------------------------
// List Lifecycle
// ---------------------------------

TLList* tl_list_create(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)
    if (allocator == NULL) {
        TLERROR("Attempted to use a NULL TLAllocator")
        TL_PROFILER_POP_WITH(NULL)
    }

    TLList* list = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_LIST, sizeof(TLList));
    list->allocator = allocator;
    list->mutex = tl_mutex_create(allocator);
    if (!list->mutex) {
        TLFATAL("Failed to create mutex for list")
    }

    TL_PROFILER_POP_WITH(list)
}

void tl_list_destroy(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    // Clear all nodes first
    tl_list_clear(list);

    // Destroy mutex
    if (list->mutex) tl_mutex_destroy(list->mutex);

    // Free list structure if using dynamic allocator
    tl_memory_free(list->allocator, list);

    TL_PROFILER_POP
}

// ---------------------------------
// List Iterator Implementation
// ---------------------------------
typedef struct {
    TLListNode* current_node;  // Current node in list
} TLListIteratorState;

static void tl_list_iterator_check_modification(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    const TLList* list = (const TLList*)iterator->source;
    if (list->mod_count != iterator->expected_mod_count) {
        TLFATAL("Concurrent modification detected during list iteration! List was modified while being iterated (expected mod_count=%u, actual=%u)",
                iterator->expected_mod_count, list->mod_count)
    }
    TL_PROFILER_POP
}

static b8 tl_list_iterator_has_next(const TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    const TLListIteratorState* state = (const TLListIteratorState*)iterator->state;
    const b8 has_next = (state->current_node != NULL);
    TL_PROFILER_POP_WITH(has_next)
}

static void* tl_list_iterator_next(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    TLListIteratorState* state = (TLListIteratorState*)iterator->state;
    if (state->current_node == NULL) {
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
    state->current_node = list->head;
    TL_PROFILER_POP
}

static void tl_list_iterator_resync(TLIterator* iterator) {
    TL_PROFILER_PUSH_WITH("0x%p", iterator)
    TLList* list = (TLList*)iterator->source;

    // Lock list to capture current state
    tl_mutex_lock(list->mutex);

    // Update iterator with current container state
    iterator->expected_mod_count = list->mod_count;
    iterator->size = list->size;

    // Rewind to beginning
    TLListIteratorState* state = (TLListIteratorState*)iterator->state;
    state->current_node = list->head;

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

TLIterator* tl_list_iterator (TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Lock list to capture current state
    tl_mutex_lock(list->mutex);

    // Allocate iterator on list's allocator
    TLIterator* iterator = tl_memory_alloc(list->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));

    // Allocate state on list's allocator
    TLListIteratorState* state = tl_memory_alloc(list->allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLListIteratorState));

    // Initialize state
    state->current_node = list->head;

    // Initialize fail-fast iterator with data
    iterator->source = list;
    iterator->expected_mod_count = list->mod_count;
    iterator->size = list->size;
    iterator->state = state;
    iterator->allocator = list->allocator;

    // Assign function pointers
    iterator->has_modified = tl_list_iterator_check_modification;
    iterator->has_next = tl_list_iterator_has_next;
    iterator->next = tl_list_iterator_next;
    iterator->rewind = tl_list_iterator_rewind;
    iterator->resync = tl_list_iterator_resync;

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

// ---------------------------------
// Internal Helper Functions
// ---------------------------------

static TLListNode* tl_list_create_node(TLAllocator* allocator, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", allocator, data)
    TLListNode* node = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_LIST, sizeof(TLListNode));
    node->data = data;
    TL_PROFILER_POP_WITH(node)
}

static void tl_list_free_node(TLAllocator* allocator, TLListNode* node) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", allocator, node)
    tl_memory_free(allocator, node);
    TL_PROFILER_POP
}

// ---------------------------------
// List Insertion
// ---------------------------------

void tl_list_push_front(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    tl_mutex_lock(list->mutex);

    TLListNode* node = tl_list_create_node(list->allocator, data);
    if (list->head == NULL) {
        // Empty list
        list->head = node;
        list->tail = node;
    } else {
        // Insert at front
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }

    list->size++;
    list->mod_count++;
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

void tl_list_push_back(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    tl_mutex_lock(list->mutex);

    TLListNode* node = tl_list_create_node(list->allocator, data);
    if (list->tail == NULL) {
        // Empty list
        list->head = node;
        list->tail = node;
    } else {
        // Insert at back
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }

    list->size++;
    list->mod_count++;
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

void tl_list_insert_after(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    // If node is NULL, behave like push_back
    if (node == NULL) {
        tl_list_push_back(list, data);
        TL_PROFILER_POP
    }

    tl_mutex_lock(list->mutex);

    TLListNode* new_node = tl_list_create_node(list->allocator, data);
    new_node->prev = node;
    new_node->next = node->next;

    if (node->next != NULL) {
        node->next->prev = new_node;
    } else {
        // Inserting after tail
        list->tail = new_node;
    }

    node->next = new_node;
    list->size++;
    list->mod_count++;

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

void tl_list_insert_before(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    // If node is NULL, behave like push_front
    if (node == NULL) {
        tl_list_push_front(list, data);
        TL_PROFILER_POP
    }

    tl_mutex_lock(list->mutex);

    TLListNode* new_node = tl_list_create_node(list->allocator, data);
    new_node->prev = node->prev;
    new_node->next = node;

    if (node->prev != NULL) {
        node->prev->next = new_node;
    } else {
        // Inserting before head
        list->head = new_node;
    }

    node->prev = new_node;
    list->size++;
    list->mod_count++;

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

// ---------------------------------
// List Removal
// ---------------------------------

void* tl_list_pop_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(list->mutex);

    if (list->head == NULL) {
        tl_mutex_unlock(list->mutex);
        TLFATAL("Cannot pop from empty list")
    }

    TLListNode* node = list->head;
    void* data = node->data;

    list->head = node->next;

    if (list->head != NULL) {
        list->head->prev = NULL;
    } else {
        // List is now empty
        list->tail = NULL;
    }

    list->size--;
    list->mod_count++;

    tl_list_free_node(list->allocator, node);
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(data)
}

void* tl_list_pop_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(list->mutex);

    if (list->tail == NULL) {
        tl_mutex_unlock(list->mutex);
        TLFATAL("Cannot pop from empty list")
    }

    TLListNode* node = list->tail;
    void* data = node->data;
    list->tail = node->prev;

    if (list->tail != NULL) { list->tail->next = NULL; }
    else                    { list->head = NULL; }

    list->size--;
    list->mod_count++;
    tl_list_free_node(list->allocator, node);

    tl_mutex_unlock(list->mutex);
    TL_PROFILER_POP_WITH(data)
}

void* tl_list_remove(TLList* list, TLListNode* node) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, node)

    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    if (node == NULL) {
        TLERROR("Attempted to use a NULL TLListNode")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(list->mutex);

    void* data = node->data;

    // Update prev node's next pointer
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        // Removing head
        list->head = node->next;
    }

    // Update next node's prev pointer
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        // Removing tail
        list->tail = node->prev;
    }

    list->size--;
    list->mod_count++;

    tl_list_free_node(list->allocator, node);
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(data)
}

// ---------------------------------
// List Access
// ---------------------------------

void* tl_list_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(list->mutex);
    const TLListNode* node = list->head;
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(node->data)
}

void* tl_list_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_mutex_lock(list->mutex);
    const TLListNode* node = list->tail;
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(node->data)
}

// ---------------------------------
// List Queries
// ---------------------------------

u32 tl_list_size(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(0)
    }

    tl_mutex_lock(list->mutex);
    const u32 size = list->size;
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(size)
}

b8 tl_list_is_empty(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP_WITH(true)
    }

    tl_mutex_lock(list->mutex);
    const b8 empty = (list->size == 0);
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(empty)
}

void tl_list_clear(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) {
        TLERROR("Attempted to use a NULL TLList")
        TL_PROFILER_POP
    }

    tl_mutex_lock(list->mutex);

    TLListNode* node = list->head;
    while (node != NULL) {
        TLListNode* next = node->next;
        tl_list_free_node(list->allocator, node);
        node = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->mod_count++;

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}
#endif