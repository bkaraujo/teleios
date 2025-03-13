#include "teleios/core/container.h"
#include "teleios/core/memory.h"
#include "teleios/core/logger.h"

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
    TLTRACE(">> tl_list_create_node(0x%p, 0X%p)", arena, value);
    
    struct TLNode* created = tl_memory_alloc(arena, sizeof(struct TLNode), TL_MEMORY_CONTAINER_NODE);
    if (created == NULL) TLFATAL("Failed to allocate struct TLNode")
    created->payload = value;
    
    TLTRACE("<< tl_list_create_node(0x%p, 0X%p)", arena, value);
    return created;
}

TLList* tl_list_create(TLMemoryArena* arena) {
    TLTRACE(">> tl_list_create(void)")
    TLList* list = tl_memory_alloc(arena, sizeof(TLList), TL_MEMORY_CONTAINER_LIST);
    if (list == NULL) {
        TLERROR("Failed to allocate TLList")
        TLTRACE("<< tl_list_create(void)")
        return NULL;
    }

    list->arena = arena;
    list->length = 0;

    TLTRACE("<< tl_list_create(void)")
    return list;
}

u64 tl_list_length(TLList* list) {
    TLTRACE(">> tl_list_length(0x%p)", list)
    return list->length;
    TLTRACE("<< tl_list_length(0x%p)", list)
}

void* tl_list_search(TLList* list, b8 (PFN_filter)(void *value)) {
    TLTRACE(">> tl_list_search(0x%p, 0X%p)", list, PFN_filter);
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLTRACE("<< tl_list_search(0x%p, 0X%p)", list, PFN_filter)
        return NULL;
    }

    if (PFN_filter == NULL) {
        TLERROR("PFN_filter is null")
        TLTRACE("<< tl_list_search(0x%p, 0X%p)", list, PFN_filter)
        return NULL;
    } 

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (PFN_filter(node->payload)) {
            return node->payload;
        }

        node = node->next;
    }

    TLTRACE("<< tl_list_search(0x%p, 0X%p)", list, PFN_filter)
    return NULL;
}

void tl_list_foreach(TLList* list, void (PFN_handler)(void *value)) {
    TLTRACE(">> tl_list_foreach(0x%p, 0X%p)", list, PFN_handler);
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLTRACE("<< tl_list_foreach(0x%p, 0X%p)", list, PFN_handler)
        return;
    }
    if (PFN_handler == NULL) {
        TLERROR("PFN_handler is NULL")
        TLTRACE("<< tl_list_foreach(0x%p, 0X%p)", list, PFN_handler)
        return;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        PFN_handler(node->payload);
        node = node->next;
    }

    TLTRACE("<< tl_list_foreach(0x%p, 0X%p)", list, PFN_handler)
}

void tl_list_add(TLList* list, void *value) {
    TLTRACE(">> tl_list_add(0x%p, 0X%p)", list, value);
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLTRACE("<< tl_list_add(0x%p, 0X%p)", list, value)
        return;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLTRACE("<< tl_list_add(0x%p, 0X%p)", list, value)
        return;
    }

    struct TLNode* created = tl_list_create_node(list->arena, value);

    if (list->head == NULL) {
        list->head = created;
        list->tail = created;
        TLTRACE("<< tl_list_add(0x%p, 0X%p)", list, value)
        return;
    }


    if (list->head == list->tail) {
        list->tail = created;
        list->tail->previous = list->head;
        list->head->next = list->tail;
        TLTRACE("<< tl_list_add(0x%p, 0X%p)", list, value)
        return;
    }

    created->previous = list->tail;
    list->tail->next = created;
    list->tail = created;

    TLTRACE("<< tl_list_add(0x%p, 0X%p)", list, value)
}

void tl_list_remove(TLList* list, void *value) {
    TLTRACE(">> tl_list_remove(0x%p, 0X%p)", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLTRACE("<< tl_list_remove(0x%p, 0X%p)", list, value)
        return;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLTRACE("<< tl_list_remove(0x%p, 0X%p)", list, value)
        return;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLTRACE("<< tl_list_remove(0x%p, 0X%p)", list, value)
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

    TLTRACE("<< tl_list_remove(0x%p, 0X%p)", list, value)
}

b8 tl_list_after(TLList* list, void *item, void *value) {
    TLTRACE(">> tl_list_after(0x%p, 0x%p, 0X%p)", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLTRACE("<< tl_list_after(0x%p, 0x%p, 0X%p)", list, item, value)
        return FALSE;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLTRACE("<< tl_list_after(0x%p, 0x%p, 0X%p)", list, item, value)
        return FALSE;
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TLTRACE("<< tl_list_after(0x%p, 0x%p, 0X%p)", list, item, value)
        return FALSE;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLTRACE("<< tl_list_after(0x%p, 0x%p, 0X%p)", list, item, value)
        return FALSE;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->arena, value);

            created->next = node->next;
            created->previous = node;
            node->next = created;
            TLTRACE("<< tl_list_after(0x%p, 0x%p, 0X%p)", list, item, value)
            return TRUE;
        }

        node = node->next;
    }

    TLTRACE("<< tl_list_after(0x%p, 0x%p, 0X%p)", list, item, value)
    return FALSE;
}

b8 tl_list_before(TLList* list, void *item, void *value) {
    TLTRACE(">> tl_list_before(0x%p, 0x%p, 0X%p)", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLTRACE("<< tl_list_before(0x%p, 0x%p, 0X%p)", list, item, value)
        return FALSE;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLTRACE("<< tl_list_before(0x%p, 0x%p, 0X%p)", list, item, value)
        return FALSE;
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TLTRACE("<< tl_list_before(0x%p, 0x%p, 0X%p)", list, item, value)
        return FALSE;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLTRACE("<< tl_list_before(0x%p, 0x%p, 0X%p)", list, item, value)
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

            TLTRACE("<< tl_list_before(0x%p, 0x%p, 0X%p)", list, item, value)
            return TRUE;
        }

        node = node->next;
    }

    TLTRACE("<< tl_list_before(0x%p, 0x%p, 0X%p)", list, item, value)
    return FALSE;
}

b8 tl_list_contains(TLList* list, void *value) {
    TLTRACE(">> lt_list_contains(0x%p, 0X%p)", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLTRACE("<< lt_list_contains(0x%p, 0X%p)", list, value)
        return FALSE;
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLTRACE("<< lt_list_contains(0x%p, 0X%p)", list, value)
        return FALSE;
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLTRACE("<< lt_list_contains(0x%p, 0X%p)", list, value)
        return FALSE;
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == value) {
            TLTRACE("<< lt_list_contains(0x%p, 0X%p)", list, value)
            return TRUE;
        }

        node = node->next;
    }

    TLTRACE("<< lt_list_contains(0x%p, 0X%p)", list, value)
    return FALSE;

}

struct TLIterator {
    u64 length;
    struct TLNode* node;
};

TLIterator* tl_list_iterator_create(TLList* list) {
    TLTRACE(">> tl_list_iterator_create(0x%p)", list)
    TLIterator* iterator = tl_memory_alloc(list->arena, sizeof(TLIterator), TL_MEMORY_CONTAINER_ITERATOR);
    iterator->length = list->length;
    iterator->node = list->head;
    TLTRACE("<< tl_list_iterator_create(0x%p)", list)
}

void* tl_list_iterator_next(TLIterator* iterator) {
    TLTRACE(">> tl_list_iterator_next(0x%p)", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        TLTRACE("<< tl_list_iterator_next(0x%p)", iterator)
        return NULL;
    }

    void* value = iterator->node->payload;
    iterator->node = iterator->node->next;
    TLTRACE("<< tl_list_iterator_next(0x%p)", iterator)
    return value;
}