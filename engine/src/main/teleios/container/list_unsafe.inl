#ifndef __TELEIOS_CONTAINER_LIST_UNSAFE__
#define __TELEIOS_CONTAINER_LIST_UNSAFE__

#include "teleios/teleios.h"
#include "teleios/container/types.inl"

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

void tl_list_unsafe_push_front(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)

    TLListNode* node = tl_list_create_node(list->allocator, data);
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }

    list->size++;
    list->mod_count++;

    TL_PROFILER_POP
}

void tl_list_unsafe_push_back(TLList* list, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, data)

    TLListNode* node = tl_list_create_node(list->allocator, data);
    if (list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }

    list->size++;
    list->mod_count++;

    TL_PROFILER_POP
}

void tl_list_unsafe_insert_after(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)

    TLListNode* new_node = tl_list_create_node(list->allocator, data);
    new_node->prev = node;
    new_node->next = node->next;

    if (node->next != NULL) {
        node->next->prev = new_node;
    } else {
        list->tail = new_node;
    }

    node->next = new_node;
    list->size++;
    list->mod_count++;

    TL_PROFILER_POP
}

void tl_list_unsafe_insert_before(TLList* list, TLListNode* node, void* data) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, 0x%p", list, node, data)

    TLListNode* new_node = tl_list_create_node(list->allocator, data);
    new_node->prev = node->prev;
    new_node->next = node;

    if (node->prev != NULL) {
        node->prev->next = new_node;
    } else {
        list->head = new_node;
    }

    node->prev = new_node;
    list->size++;
    list->mod_count++;

    TL_PROFILER_POP
}

void* tl_list_unsafe_pop_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    TLListNode* node = list->head;
    void* data = node->data;

    list->head = node->next;
    if (list->head != NULL) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL;
    }

    list->size--;
    list->mod_count++;
    tl_list_free_node(list->allocator, node);

    TL_PROFILER_POP_WITH(data)
}

void* tl_list_unsafe_pop_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

    TLListNode* node = list->tail;
    void* data = node->data;
    list->tail = node->prev;

    if (list->tail != NULL) {
        list->tail->next = NULL;
    } else {
        list->head = NULL;
    }

    list->size--;
    list->mod_count++;
    tl_list_free_node(list->allocator, node);

    TL_PROFILER_POP_WITH(data)
}

void* tl_list_unsafe_remove(TLList* list, TLListNode* node) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", list, node)

    void* data = node->data;

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    list->size--;
    list->mod_count++;
    tl_list_free_node(list->allocator, node);

    TL_PROFILER_POP_WITH(data)
}

void* tl_list_unsafe_front(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    TL_PROFILER_POP_WITH(list->head->data)
}

void* tl_list_unsafe_back(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    TL_PROFILER_POP_WITH(list->tail->data)
}

u32 tl_list_unsafe_size(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    TL_PROFILER_POP_WITH(list->size)
}

b8 tl_list_unsafe_is_empty(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)
    TL_PROFILER_POP_WITH(list->size == 0)
}

void tl_list_unsafe_clear(TLList* list) {
    TL_PROFILER_PUSH_WITH("0x%p", list)

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

    TL_PROFILER_POP
}

#endif
