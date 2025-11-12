#include "teleios/teleios.h"
#include "teleios/memory_types.inl"
#include "teleios/container_types.inl"

// ---------------------------------
// List Lifecycle
// ---------------------------------

TLList* tl_list_create(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)

    if (allocator == NULL) TLFATAL("allocator is NULL")

    TLList* list = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_LIST, sizeof(TLList));

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
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
        TLWARN("Attempted to destroy NULL list")
        TL_PROFILER_POP
    }

    // Clear all nodes first
    tl_list_clear(list);

    // Destroy mutex
    if (list->mutex) tl_mutex_destroy(list->mutex);

    // Free list structure if using dynamic allocator
    if (list->allocator->type == TL_ALLOCATOR_DYNAMIC) {
        tl_memory_free(list->allocator, list);
    }

    TL_PROFILER_POP
}

TLIterator* tl_list_iterator (TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLERROR("list is NULL")
        TL_PROFILER_POP_WITH(NULL)
    }

    // Lock list to create thread-safe snapshot
    tl_mutex_lock(list->mutex);

    TLAllocator* allocator = tl_memory_allocator_create(TL_ALLOCATOR_LINEAR, TL_KIBI_BYTES(4));
    TLIterator* iterator = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(TLIterator));
    iterator->allocator = allocator;
    iterator->current = 0;

    iterator->size = list->size;

    // Empty list - create empty iterator
    if (iterator->size == 0) {
        tl_mutex_unlock(list->mutex);
        iterator->items = NULL;

        TL_PROFILER_POP_WITH(iterator)
    }

    // Allocate iterator structure
    iterator->items = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_ITERATOR, sizeof(void*) * iterator->size);

    // Copy all data pointers into contiguous array
    // This is the only part that needs the lock - after this, iteration is lock-free
    const TLListNode* node = list->head;
    u32 index = 0;

    while (node != NULL && index < iterator->size) {
        iterator->items[index++] = node->data;
        node = node->next;
    }

    // Unlock - snapshot is complete
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(iterator)
}

// ---------------------------------
// Internal Helper Functions
// ---------------------------------

static TLListNode* tl_list_create_node(TLAllocator* allocator, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", allocator, data)

    TLListNode* node = tl_memory_alloc(allocator, TL_MEMORY_CONTAINER_LIST, sizeof(TLListNode));
    if (node == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }

    node->data = data;
    node->prev = NULL;
    node->next = NULL;

    TL_PROFILER_POP_WITH(node)
}

static void tl_list_free_node(TLAllocator* allocator, TLListNode* node) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", allocator, node)

    if (allocator->type == TL_ALLOCATOR_DYNAMIC) {
        tl_memory_free(allocator, node);
    }

    TL_PROFILER_POP
}

// ---------------------------------
// List Insertion
// ---------------------------------

void tl_list_push_front(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)

    if (list == NULL) TLFATAL("list is NULL")

    tl_mutex_lock(list->mutex);

    TLListNode* node = tl_list_create_node(list->allocator, data);
    if (node == NULL) {
        tl_mutex_unlock(list->mutex);
        TLERROR("Failed to create node")
        TL_PROFILER_POP
    }

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
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

void tl_list_push_back(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)

    if (list == NULL) TLFATAL("list is NULL")

    tl_mutex_lock(list->mutex);

    TLListNode* node = tl_list_create_node(list->allocator, data);
    if (node == NULL) {
        tl_mutex_unlock(list->mutex);
        TLERROR("Failed to create node")
        TL_PROFILER_POP
    }

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
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

void tl_list_insert_after(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)

    if (list == NULL) TLFATAL("list is NULL")

    // If node is NULL, behave like push_back
    if (node == NULL) {
        tl_list_push_back(list, data);
        TL_PROFILER_POP
    }

    tl_mutex_lock(list->mutex);

    TLListNode* new_node = tl_list_create_node(list->allocator, data);
    if (new_node == NULL) {
        tl_mutex_unlock(list->mutex);
        TLERROR("Failed to create node")
        TL_PROFILER_POP
    }

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

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

void tl_list_insert_before(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)

    if (list == NULL) TLFATAL("list is NULL")

    // If node is NULL, behave like push_front
    if (node == NULL) {
        tl_list_push_front(list, data);
        TL_PROFILER_POP
    }

    tl_mutex_lock(list->mutex);

    TLListNode* new_node = tl_list_create_node(list->allocator, data);
    if (new_node == NULL) {
        tl_mutex_unlock(list->mutex);
        TLERROR("Failed to create node")
        TL_PROFILER_POP
    }

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

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}

// ---------------------------------
// List Removal
// ---------------------------------

void* tl_list_pop_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) TLFATAL("list is NULL")

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

    tl_list_free_node(list->allocator, node);
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(data)
}

void* tl_list_pop_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) TLFATAL("list is NULL")

    tl_mutex_lock(list->mutex);

    if (list->tail == NULL) {
        tl_mutex_unlock(list->mutex);
        TLFATAL("Cannot pop from empty list")
    }

    TLListNode* node = list->tail;
    void* data = node->data;

    list->tail = node->prev;

    if (list->tail != NULL) {
        list->tail->next = NULL;
    } else {
        // List is now empty
        list->head = NULL;
    }

    list->size--;

    tl_list_free_node(list->allocator, node);
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(data)
}

void* tl_list_remove(TLList* list, TLListNode* node) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, node)

    if (list == NULL) TLFATAL("list is NULL")
    if (node == NULL) TLFATAL("node is NULL")

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

    tl_list_free_node(list->allocator, node);
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(data)
}

// ---------------------------------
// List Access
// ---------------------------------

void* tl_list_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) TL_PROFILER_POP_WITH(NULL)

    tl_mutex_lock(list->mutex);
    const TLListNode* node = list->head;
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(node->data)
}

void* tl_list_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) TL_PROFILER_POP_WITH(NULL)

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
    if (list == NULL) TL_PROFILER_POP_WITH(0)

    tl_mutex_lock(list->mutex);
    const u32 size = list->size;
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(size)
}

b8 tl_list_is_empty(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    if (list == NULL) TL_PROFILER_POP_WITH(true)

    tl_mutex_lock(list->mutex);
    const b8 empty = (list->size == 0);
    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP_WITH(empty)
}

void tl_list_clear(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    if (list == NULL) {
        TLWARN("Attempted to clear NULL list")
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

    tl_mutex_unlock(list->mutex);

    TL_PROFILER_POP
}