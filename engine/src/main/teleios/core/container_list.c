#include "teleios/core.h"

struct TLNode {
    void *payload;
    struct TLNode* next;
    struct TLNode* previous;
};

struct TLList {
    u64 length;
    struct TLNode* head;
    struct TLNode* tail;
    TLMemoryArena* arena;
} ;

static struct TLNode* tl_list_create_node(TLMemoryArena *arena, void *value) {
    TLSTACKPUSHA("0x%p, 0X%p", arena, value)
    
    struct TLNode* created = tl_memory_alloc(arena, sizeof(struct TLNode), TL_MEMORY_CONTAINER_NODE);
    if (created == NULL) TLFATAL("Failed to allocate struct TLNode")
    created->payload = value;
    
    TLSTACKPOP
    return created;
}

TLList* tl_list_create(TLMemoryArena* arena) {
    TLSTACKPUSHA("0x%p", arena)
    TLList* list = tl_memory_alloc(arena, sizeof(TLList), TL_MEMORY_CONTAINER_LIST);
    if (list == NULL) {
        TLERROR("Failed to allocate TLList")
        TLSTACKPOP
        return NULL;
    }

    list->arena = arena;
    list->length = 0;

    TLSTACKPOP
    return list;
}

u64 tl_list_length(TLList* list) {
    TLSTACKPUSHA("0x%p", list)
    const u64 length = list->length;
    TLSTACKPOP
    return length;
}

void* tl_list_search(TLList* list, b8 (*PFN_filter)(void *value)) {
    TLSTACKPUSHA("0x%p, 0x%p", list, PFN_filter)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
        return NULL;
    }

    if (PFN_filter == NULL) {
        TLERROR("PFN_filter is null")
        TLSTACKPOP
        return NULL;
    } 

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (PFN_filter(node->payload)) {
            return node->payload;
        }

        node = node->next;
    }

    TLSTACKPOP
    return NULL;
}

void tl_list_foreach(TLList* list, void (*PFN_handler)(void *value)) {
    TLSTACKPUSHA("0x%p, 0x%p", list, PFN_handler)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
        return;
    }
    if (PFN_handler == NULL) {
        TLERROR("PFN_handler is NULL")
        TLSTACKPOP
        return;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        PFN_handler(node->payload);
        node = node->next;
    }

    TLSTACKPOP
}

void tl_list_add(TLList* list, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
        return;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOP
        return;
    }

    struct TLNode* created = tl_list_create_node(list->arena, value);

    if (list->head == NULL) {
        list->length++;
        list->head = created;
        list->tail = created;
        TLSTACKPOP
        return;
    }


    if (list->head == list->tail) {
        list->length++;
        list->tail = created;
        list->tail->previous = list->head;
        list->head->next = list->tail;
        TLSTACKPOP
        return;
    }

    list->length++;
    created->previous = list->tail;
    list->tail->next = created;
    list->tail = created;

    TLSTACKPOP
}

void tl_list_remove(TLList* list, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
        return;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOP
        return;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOP
        return;
    }

    if (list->length == 1) {
        if (list->head->payload == value) {
            list->length--;
            list->head = NULL;
            list->tail = NULL;
        } else {
            TLWARN("The TLList 0x%p does not contain 0x%p", list, value)
        }

        TLSTACKPOP
        return;
    }

    if (list->tail->payload == value) {
        list->length--;
        list->tail->previous->next = NULL;
        list->tail = list->tail->previous;

        TLSTACKPOP
        return;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == value) {
            struct TLNode* previous = node->previous;
            struct TLNode* next = node->next;

            previous->next = next;
            next->previous = previous;
            break;
        }

        node = node->next;
    }

    TLSTACKPOP
}

b8 tl_list_after(TLList* list, void *item, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
        return FALSE;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOP
        return FALSE;
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TLSTACKPOP
        return FALSE;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOP
        return FALSE;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->arena, value);

            created->next = node->next;
            created->previous = node;
            node->next = created;
            TLSTACKPOP
            return TRUE;
        }

        node = node->next;
    }

    TLSTACKPOP
    return FALSE;
}

b8 tl_list_before(TLList* list, void *item, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
        return FALSE;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOP
        return FALSE;
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TLSTACKPOP
        return FALSE;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOP
        return FALSE;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->arena, value);

            created->next = node;
            created->previous = node->previous;
            node->previous->next = created;
            node->previous = created;

            TLSTACKPOP
            return TRUE;
        }

        node = node->next;
    }

    TLSTACKPOP
    return FALSE;
}

b8 tl_list_contains(TLList* list, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
        return FALSE;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOP
        return FALSE;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOP
        return FALSE;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == value) {
            TLSTACKPOP
            return TRUE;
        }

        node = node->next;
    }

    TLSTACKPOP
    return FALSE;

}

struct TLIterator {
    u64 length;
    struct TLNode* node;
};

TLIterator* tl_list_iterator_create(TLList* list) {
    TLSTACKPUSHA("0x%p", list)
    TLIterator* iterator = tl_memory_alloc(list->arena, sizeof(TLIterator), TL_MEMORY_CONTAINER_ITERATOR);
    iterator->length = list->length;
    iterator->node = list->head;
    TLSTACKPOP
    return iterator;
}

void* tl_list_iterator_next(TLIterator* iterator) {
    TLSTACKPUSHA("0x%p", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        TLSTACKPOP
        return NULL;
    }

    void* value = iterator->node->payload;
    iterator->node = iterator->node->next;
    TLSTACKPOP
    return value;
}