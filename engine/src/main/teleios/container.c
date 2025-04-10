#include "teleios/core.h"
#include "teleios/runtime.h"

// #####################################################################################################################
//
//                                                     ITERATOR
//
// #####################################################################################################################
struct TLIterator {u64 capacity; u64 length; void* node; void* (*next)(TLIterator*); };
void* tl_iterator_next(TLIterator* iterator) {
    K_FRAME_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        K_FRAME_POP_WITH(NULL)
    }

    void *next = iterator->next(iterator);
    K_FRAME_POP_WITH(next)
}
// #####################################################################################################################
//
//                                                     LIST
//
// #####################################################################################################################
struct TLNode {
    void *payload;
    struct TLNode* next;
    struct TLNode* previous;
};

struct TLList {
    u64 length;
    struct TLNode* head;
    struct TLNode* tail;
    KAllocator *allocator;
} ;

static struct TLNode* tl_list_create_node(KAllocator *allocator, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0X%p", allocator, value)

    struct TLNode* created = k_memory_allocator_alloc(allocator, sizeof(struct TLNode), TL_MEMORY_CONTAINER_NODE);
    if (created == NULL) KFATAL("Failed to allocate struct TLNode")
    created->payload = value;

    K_FRAME_POP_WITH(created)
}

TLList* tl_list_create(KAllocator *allocator) {
    K_FRAME_PUSH_WITH("0x%p", allocator)
    TLList* list = k_memory_allocator_alloc(allocator, sizeof(TLList), TL_MEMORY_CONTAINER_LIST);
    if (list == NULL) {
        KERROR("Failed to allocate TLList")
        K_FRAME_POP_WITH(NULL)
    }

    list->allocator = allocator;
    list->length = 0;

    K_FRAME_POP_WITH(list)
}

u64 tl_list_length(TLList* list) {
    K_FRAME_PUSH_WITH("0x%p", list)
    K_FRAME_POP_WITH(list->length)
}

void* tl_list_search(TLList* list, b8 (*PFN_filter)(void *value)) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", list, PFN_filter)
    if (list == NULL) {
        KERROR("TLList is NULL")
        K_FRAME_POP_WITH(NULL)
    }

    if (PFN_filter == NULL) {
        KERROR("PFN_filter is null")
        K_FRAME_POP_WITH(NULL)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (PFN_filter(node->payload)) {
            K_FRAME_POP_WITH(node->payload);
        }

        node = node->next;
    }

    K_FRAME_POP_WITH(NULL)
}

void tl_list_foreach(TLList* list, void (*PFN_handler)(void *value)) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", list, PFN_handler)
    if (list == NULL) {
        KERROR("TLList is NULL")
        K_FRAME_POP
    }
    if (PFN_handler == NULL) {
        KERROR("PFN_handler is NULL")
        K_FRAME_POP
    }

    const struct TLNode* node = list->head;
    while (node != NULL) {
        PFN_handler(node->payload);
        node = node->next;
    }

    K_FRAME_POP
}

void tl_list_add(TLList* list, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", list, value)
    if (list == NULL) {
        KERROR("TLList is NULL")
        K_FRAME_POP
    }

    if (value == NULL) {
        KERROR("value is NULL")
        K_FRAME_POP
    }

    struct TLNode* created = tl_list_create_node(list->allocator, value);

    if (list->head == NULL) {
        list->length++;
        list->head = created;
        list->tail = created;
        K_FRAME_POP
    }

    if (list->head == list->tail) {
        list->length++;
        list->tail = created;
        list->tail->previous = list->head;
        list->head->next = list->tail;
        K_FRAME_POP
    }

    list->length++;
    created->previous = list->tail;
    list->tail->next = created;
    list->tail = created;

    K_FRAME_POP
}

void tl_list_remove(TLList* list, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", list, value)
    if (list == NULL) {
        KERROR("TLList is NULL")
        K_FRAME_POP
    }

    if (list->head == NULL) {
        KERROR("TLList is empty")
        K_FRAME_POP
    }

    if (value == NULL) {
        KERROR("value is NULL")
        K_FRAME_POP
    }

    if (list->length == 1) {
        if (list->head->payload == value) {
            list->length--;
            list->head = NULL;
            list->tail = NULL;
        } else {
            KWARN("The TLList 0x%p does not contain 0x%p", list, value)
        }

        K_FRAME_POP
    }

    if (list->tail->payload == value) {
        list->length--;
        list->tail->previous->next = NULL;
        list->tail = list->tail->previous;

        K_FRAME_POP
    }

    const struct TLNode* node = list->head;
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

    K_FRAME_POP
}

b8 tl_list_after(TLList* list, void *item, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        KERROR("TLList is NULL")
        K_FRAME_POP_WITH(false)
    }

    if (list->head == NULL) {
        KERROR("TLList is empty")
        K_FRAME_POP_WITH(false)
    }

    if (item == NULL) {
        KERROR("item is NULL")
        K_FRAME_POP_WITH(false)
    }

    if (value == NULL) {
        KERROR("value is NULL")
        K_FRAME_POP_WITH(false)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->allocator, value);

            created->next = node->next;
            created->previous = node;
            node->next = created;
            K_FRAME_POP_WITH(true)
        }

        node = node->next;
    }

    K_FRAME_POP_WITH(false)
}

b8 tl_list_before(TLList* list, void *item, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        KERROR("TLList is NULL")
        K_FRAME_POP_WITH(false)
    }

    if (list->head == NULL) {
        KERROR("TLList is empty")
        K_FRAME_POP_WITH(false)
    }

    if (item == NULL) {
        KERROR("item is NULL")
        K_FRAME_POP_WITH(false)
    }

    if (value == NULL) {
        KERROR("value is NULL")
        K_FRAME_POP_WITH(false)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->allocator, value);

            created->next = node;
            created->previous = node->previous;
            node->previous->next = created;
            node->previous = created;
            K_FRAME_POP_WITH(true)
        }

        node = node->next;
    }

    K_FRAME_POP_WITH(false)
}

b8 tl_list_contains(TLList* list, void *value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", list, value)
    if (list == NULL) {
        KERROR("TLList is NULL")
        K_FRAME_POP_WITH(false)
    }

    if (list->head == NULL) {
        KERROR("TLList is empty")
        K_FRAME_POP_WITH(false)
    }

    if (value == NULL) {
        KERROR("value is NULL")
        K_FRAME_POP_WITH(false)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == value) {
            K_FRAME_POP_WITH(true)
        }

        node = node->next;
    }

    K_FRAME_POP_WITH(false)

}

static void* tl_list_iterator_next(TLIterator *iterator) {
    K_FRAME_PUSH_WITH("0x%p", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        K_FRAME_POP_WITH(NULL)
    }

    if (iterator->length == iterator->capacity) {
        K_FRAME_POP_WITH(NULL)
    }

    struct TLNode* node = iterator->node;
    void* value = node->payload;
    iterator->node = node->next;
    iterator->length++;

    K_FRAME_POP_WITH(value)
}

TLIterator* tl_list_iterator_create(KAllocator *allocator, TLList* list) {
    K_FRAME_PUSH_WITH("0x%p", list)
    TLIterator* iterator = k_memory_allocator_alloc(allocator == NULL ? list->allocator : allocator, sizeof(TLIterator), TL_MEMORY_CONTAINER_ITERATOR);
    iterator->length = 0;
    iterator->capacity = list->length;
    iterator->node = list->head;
    iterator->next = tl_list_iterator_next;

    K_FRAME_POP_WITH(iterator)
}
// #####################################################################################################################
//
//                                                     STACK
//
// #####################################################################################################################
TLStack* tl_stack_create(KAllocator *allocator)  {
    K_FRAME_PUSH_WITH("0x%p", allocator)
    TLStack* stack = k_memory_allocator_alloc(allocator, sizeof(TLStack), TL_MEMORY_CONTAINER_STACK);
    stack = tl_list_create(allocator);
    K_FRAME_POP_WITH(stack)
}

void tl_stack_push(TLStack* stack, void* value) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", stack, value)
    tl_list_add(stack, value);
    K_FRAME_POP
}

void* tl_stack_peek(TLStack* stack) {
    K_FRAME_PUSH_WITH("0x%p", stack)
    void* value = NULL;
    TLIterator* iterator = tl_list_iterator_create(NULL, stack);
    void* next = tl_iterator_next(iterator);
    while (next != NULL) { value = next; next = tl_iterator_next(iterator); }
    K_FRAME_POP_WITH(value)
}

void* tl_stack_pop(TLStack* stack) {
    K_FRAME_PUSH_WITH("0x%p", stack)
    void* value = tl_stack_peek(stack);
    tl_list_remove(stack, value);
    K_FRAME_POP_WITH(value)
}

u64 tl_stack_length(TLStack* stack) {
    K_FRAME_PUSH_WITH("0x%p", stack)
    K_FRAME_POP_WITH(tl_list_length(stack))
}

TLIterator* tl_stack_iterator_create(TLStack* stack) {
    K_FRAME_PUSH_WITH("0x%p", stack)
    TLIterator* iterator = k_memory_allocator_alloc(stack->allocator, sizeof(TLIterator), TL_MEMORY_CONTAINER_ITERATOR);
    iterator->length = 0;
    iterator->capacity = stack->length;
    iterator->node = stack->head;
    iterator->next = tl_list_iterator_next;
    K_FRAME_POP_WITH(iterator)
}
// #####################################################################################################################
//
//                                                     MAP
//
// #####################################################################################################################
struct TLMapEntry { TLString *key; void *payload; };
struct TLMap { KAllocator *allocator; struct TLMapEntry *values; u16 size; u16 length; };

TLMap* tl_map_create(KAllocator *allocator) {
    K_FRAME_PUSH_WITH("0x%p", allocator)
    if (allocator == NULL) KFATAL("allocator is NULL")

    TLMap *map = k_memory_allocator_alloc(allocator, sizeof(TLMap), TL_MEMORY_CONTAINER_MAP);
    map->size = 10;
    map->length = 0;
    map->allocator = allocator;

    K_FRAME_POP_WITH(map)
}

void tl_map_put(TLMap* map, const char *key, void *value) {
    K_FRAME_PUSH_WITH("0x%p, %s, 0x%p", map, key, value)
    if (map == NULL) KFATAL("TLMap is NULL")
    if (key == NULL) KFATAL("key is NULL")
    if (value == NULL) KFATAL("value is NULL")

    if (map->values == NULL) {
        KTRACE("TLMap 0x%p initialized with capacity of %d", map, map->size)
        map->values = k_memory_allocator_alloc(map->allocator, map->size * sizeof(struct TLMapEntry), TL_MEMORY_CONTAINER_NODE);
    }

    if (map->length >= map->size) {
        u16 new_size = (u16)((f32)map->size * 1.75f) + 1;
        KTRACE("TLMap 0x%p resized from %d to %d capacity", map, map->size, new_size)
        void *new_values = k_memory_allocator_alloc(map->allocator, new_size * sizeof(struct TLMapEntry), TL_MEMORY_CONTAINER_NODE);
        k_memory_copy(new_values, map->values, map->size * sizeof(struct TLMapEntry));
        map->size = new_size;
        map->values = new_values;
    }

    for (u16 i = 0; i < map->length; i++) {
        struct TLMapEntry *entry = &map->values[i];
        if (entry->key == NULL) continue;
        if (tl_string_equals(entry->key, key)) {
            entry->payload = value;
            K_FRAME_POP
        }
    }

    map->values[map->length].key = tl_string_clone(map->allocator, key);
    map->values[map->length].payload = value;
    map->length++;
    K_FRAME_POP
}

void* tl_map_get(TLMap* map, const char *key) {
    K_FRAME_PUSH_WITH("0x%p, %", map, key)
    if (map == NULL) KFATAL("TLMap is NULL")
    if (key == NULL) KFATAL("key is NULL")

    for (u16 i = 0; i < map->size; i++) {
        if (tl_string_equals(map->values[i].key, key)) {
            K_FRAME_POP_WITH(map->values[i].payload)
        }
    }

    K_FRAME_POP_WITH(NULL)
}

u16 tl_map_length(TLMap* map) {
    K_FRAME_PUSH_WITH("0x%p", map)
    const u16 length = map->length;
    K_FRAME_POP_WITH(length)
}

b8 tl_map_contains(TLMap* map, const char *key) {
    K_FRAME_PUSH_WITH("0x%p, %", map, key)
    b8 contains = tl_map_get(map, key) != NULL;
    K_FRAME_POP_WITH(contains)
}

void tl_map_remove(TLMap* map, const char *key) {
    K_FRAME_PUSH_WITH("0x%p, %", map, key)
    if (map == NULL) KFATAL("TLMap is NULL")
    if (key == NULL) KFATAL("key is NULL")

    for (u16 i = 0; i < map->size; i++) {
        if (tl_string_equals(map->values[i].key, key)) {
            k_memory_copy(
                map->values + i,
                map->values + i + 1,
                sizeof(struct TLMapEntry) * (map->size - map->length - 1)
            );

            map->length--;
            k_memory_set(
                map->values + map->length,
                0,
                sizeof(struct TLMapEntry) * (map->size - map->length)
            );
        }
    }

    K_FRAME_POP
}

TLIterator* tl_map_keys(KAllocator *allocator, TLMap* map) {
    K_FRAME_PUSH_WITH("0x%p", map)
    if (map == NULL) KFATAL("TLMap is NULL")

    TLList *keys = tl_list_create(map->allocator);
    if (map->length == 0) K_FRAME_POP_WITH(NULL)

    for (u16 i = 0; i < map->length; ++i) {
        tl_list_add(keys, map->values[i].key);
    }

    TLIterator *it = tl_list_iterator_create(allocator, keys);
    K_FRAME_POP_WITH(it)
}