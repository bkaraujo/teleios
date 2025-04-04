#include "teleios/core.h"
#include "teleios/runtime.h"

// #####################################################################################################################
//
//                                                     ITERATOR
//
// #####################################################################################################################
struct TLIterator {u64 capacity; u64 length; void* node; void* (*next)(TLIterator*); };
void* tl_iterator_next(TLIterator* iterator) {
    TL_STACK_PUSHA("0x%p", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        TL_STACK_POPV(NULL)
    }

    void *next = iterator->next(iterator);
    TL_STACK_POPV(next)
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
    TLMemoryArena *arena;
} ;

static struct TLNode* tl_list_create_node(TLMemoryArena *arena, void *value) {
    TL_STACK_PUSHA("0x%p, 0X%p", arena, value)

    struct TLNode* created = tl_memory_alloc(arena, sizeof(struct TLNode), TL_MEMORY_CONTAINER_NODE);
    if (created == NULL) TLFATAL("Failed to allocate struct TLNode")
    created->payload = value;

    TL_STACK_POPV(created)
}

TLList* tl_list_create(TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)
    TLList* list = tl_memory_alloc(arena, sizeof(TLList), TL_MEMORY_CONTAINER_LIST);
    if (list == NULL) {
        TLERROR("Failed to allocate TLList")
        TL_STACK_POPV(NULL)
    }

    list->arena = arena;
    list->length = 0;

    TL_STACK_POPV(list)
}

u64 tl_list_length(TLList* list) {
    TL_STACK_PUSHA("0x%p", list)
    TL_STACK_POPV(list->length)
}

void* tl_list_search(TLList* list, b8 (*PFN_filter)(void *value)) {
    TL_STACK_PUSHA("0x%p, 0x%p", list, PFN_filter)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TL_STACK_POPV(NULL)
    }

    if (PFN_filter == NULL) {
        TLERROR("PFN_filter is null")
        TL_STACK_POPV(NULL)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (PFN_filter(node->payload)) {
            TL_STACK_POPV(node->payload);
        }

        node = node->next;
    }

    TL_STACK_POPV(NULL)
}

void tl_list_foreach(TLList* list, void (*PFN_handler)(void *value)) {
    TL_STACK_PUSHA("0x%p, 0x%p", list, PFN_handler)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TL_STACK_POP
    }
    if (PFN_handler == NULL) {
        TLERROR("PFN_handler is NULL")
        TL_STACK_POP
    }

    const struct TLNode* node = list->head;
    while (node != NULL) {
        PFN_handler(node->payload);
        node = node->next;
    }

    TL_STACK_POP
}

void tl_list_add(TLList* list, void *value) {
    TL_STACK_PUSHA("0x%p, 0x%p", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TL_STACK_POP
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TL_STACK_POP
    }

    struct TLNode* created = tl_list_create_node(list->arena, value);

    if (list->head == NULL) {
        list->length++;
        list->head = created;
        list->tail = created;
        TL_STACK_POP
    }

    if (list->head == list->tail) {
        list->length++;
        list->tail = created;
        list->tail->previous = list->head;
        list->head->next = list->tail;
        TL_STACK_POP
    }

    list->length++;
    created->previous = list->tail;
    list->tail->next = created;
    list->tail = created;

    TL_STACK_POP
}

void tl_list_remove(TLList* list, void *value) {
    TL_STACK_PUSHA("0x%p, 0x%p", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TL_STACK_POP
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TL_STACK_POP
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TL_STACK_POP
    }

    if (list->length == 1) {
        if (list->head->payload == value) {
            list->length--;
            list->head = NULL;
            list->tail = NULL;
        } else {
            TLWARN("The TLList 0x%p does not contain 0x%p", list, value)
        }

        TL_STACK_POP
    }

    if (list->tail->payload == value) {
        list->length--;
        list->tail->previous->next = NULL;
        list->tail = list->tail->previous;

        TL_STACK_POP
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

    TL_STACK_POP
}

b8 tl_list_after(TLList* list, void *item, void *value) {
    TL_STACK_PUSHA("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TL_STACK_POPV(false)
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TL_STACK_POPV(false)
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TL_STACK_POPV(false)
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TL_STACK_POPV(false)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->arena, value);

            created->next = node->next;
            created->previous = node;
            node->next = created;
            TL_STACK_POPV(true)
        }

        node = node->next;
    }

    TL_STACK_POPV(false)
}

b8 tl_list_before(TLList* list, void *item, void *value) {
    TL_STACK_PUSHA("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TL_STACK_POPV(false)
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TL_STACK_POPV(false)
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TL_STACK_POPV(false)
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TL_STACK_POPV(false)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->arena, value);

            created->next = node;
            created->previous = node->previous;
            node->previous->next = created;
            node->previous = created;
            TL_STACK_POPV(true)
        }

        node = node->next;
    }

    TL_STACK_POPV(false)
}

b8 tl_list_contains(TLList* list, void *value) {
    TL_STACK_PUSHA("0x%p, 0x%p", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TL_STACK_POPV(false)
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TL_STACK_POPV(false)
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TL_STACK_POPV(false)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == value) {
            TL_STACK_POPV(true)
        }

        node = node->next;
    }

    TL_STACK_POPV(false)

}

static void* tl_list_iterator_next(TLIterator *iterator) {
    TL_STACK_PUSHA("0x%p", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        TL_STACK_POPV(NULL)
    }

    if (iterator->length == iterator->capacity) {
        TL_STACK_POPV(NULL)
    }

    struct TLNode* node = iterator->node;
    void* value = node->payload;
    iterator->node = node->next;
    iterator->length++;

    TL_STACK_POPV(value)
}

TLIterator* tl_list_iterator_create(TLList* list) {
    TL_STACK_PUSHA("0x%p", list)
    TLIterator* iterator = tl_memory_alloc(list->arena, sizeof(TLIterator), TL_MEMORY_CONTAINER_ITERATOR);
    iterator->length = 0;
    iterator->capacity = list->length;
    iterator->node = list->head;
    iterator->next = tl_list_iterator_next;

    TL_STACK_POPV(iterator)
}
// #####################################################################################################################
//
//                                                     STACK
//
// #####################################################################################################################
TLStack* tl_stack_create(TLMemoryArena *arena)  {
    TL_STACK_PUSHA("0x%p", arena)
    TLStack* stack = tl_memory_alloc(arena, sizeof(TLStack), TL_MEMORY_CONTAINER_STACK);
    stack = tl_list_create(arena);
    TL_STACK_POPV(stack)
}

void tl_stack_push(TLStack* stack, void* value) {
    TL_STACK_PUSHA("0x%p, 0x%p", stack, value)
    tl_list_add(stack, value);
    TL_STACK_POP
}

void* tl_stack_peek(TLStack* stack) {
    TL_STACK_PUSHA("0x%p", stack)
    void* value = NULL;
    TLIterator* iterator = tl_list_iterator_create(stack);
    void* next = tl_iterator_next(iterator);
    while (next != NULL) { value = next; next = tl_iterator_next(iterator); }
    TL_STACK_POPV(value)
}

void* tl_stack_pop(TLStack* stack) {
    TL_STACK_PUSHA("0x%p", stack)
    void* value = tl_stack_peek(stack);
    tl_list_remove(stack, value);
    TL_STACK_POPV(value)
}

u64 tl_stack_length(TLStack* stack) {
    TL_STACK_PUSHA("0x%p", stack)
    TL_STACK_POPV(tl_list_length(stack))
}

TLIterator* tl_stack_iterator_create(TLStack* stack) {
    TL_STACK_PUSHA("0x%p", stack)
    TLIterator* iterator = tl_memory_alloc(stack->arena, sizeof(TLIterator), TL_MEMORY_CONTAINER_ITERATOR);
    iterator->length = 0;
    iterator->capacity = stack->length;
    iterator->node = stack->head;
    iterator->next = tl_list_iterator_next;
    TL_STACK_POPV(iterator)
}

struct TLMapEntry { TLString *key; void *payload; };
struct TLMap { TLMemoryArena *arena; struct TLMapEntry *values; u16 size; u16 length; };

// #####################################################################################################################
//
//                                                     MAP
//
// #####################################################################################################################
TLMap* tl_map_create(TLMemoryArena *arena) {
    TL_STACK_PUSHA("0x%p", arena)
    if (arena == NULL) TLFATAL("Arena is NULL")

    TLMap *map = tl_memory_alloc(arena, sizeof(TLMap), TL_MEMORY_CONTAINER_MAP);
    map->size = 10;
    map->length = 0;
    map->arena = arena;

    TL_STACK_POPV(map)
}

void tl_map_put(TLMap* map, const char *key, void *value) {
    TL_STACK_PUSHA("0x%p, %s, 0x%p", map, key, value)
    if (map == NULL) TLFATAL("TLMap is NULL")
    if (key == NULL) TLFATAL("key is NULL")
    if (value == NULL) TLFATAL("value is NULL")

    if (map->values == NULL) {
        TLTRACE("TLMap 0x%p initialized with capacity of %d", map, map->size)
        map->values = tl_memory_alloc(map->arena, map->size * sizeof(struct TLMapEntry), TL_MEMORY_CONTAINER_NODE);
    }

    if (map->length >= map->size) {
        u16 new_size = (u16)((f32)map->size * 1.75f) + 1;
        TLTRACE("TLMap 0x%p resized from %d to %d capacity", map, map->size, new_size)
        void *new_values = tl_memory_alloc(map->arena, new_size * sizeof(struct TLMapEntry), TL_MEMORY_CONTAINER_NODE);
        tl_memory_copy(new_values, map->values, map->size * sizeof(struct TLMapEntry));
        map->size = new_size;
        map->values = new_values;
    }

    for (u16 i = 0; i < map->length; i++) {
        struct TLMapEntry *entry = &map->values[i];
        if (entry->key == NULL) continue;
        if (tl_string_equals(entry->key, key)) {
            entry->payload = value;
            TL_STACK_POP
        }
    }

    map->values[map->length].key = tl_string_clone(map->arena, key);
    map->values[map->length].payload = value;
    map->length++;
    TL_STACK_POP
}

void* tl_map_get(TLMap* map, const char *key) {
    TL_STACK_PUSHA("0x%p, %", map, key)
    if (map == NULL) TLFATAL("TLMap is NULL")
    if (key == NULL) TLFATAL("key is NULL")

    for (u16 i = 0; i < map->size; i++) {
        if (tl_string_equals(map->values[i].key, key)) {
            TL_STACK_POPV(map->values[i].payload)
        }
    }

    TL_STACK_POPV(NULL)
}
u16 tl_map_length(TLMap* map) {
    TL_STACK_PUSHA("0x%p", map)
    const u16 length = map->length;
    TL_STACK_POPV(length)
}

b8 tl_map_contains(TLMap* map, const char *key) {
    TL_STACK_PUSHA("0x%p, %", map, key)
    b8 contains = tl_map_get(map, key) != NULL;
    TL_STACK_POPV(contains)
}

void tl_map_remove(TLMap* map, const char *key) {
    TL_STACK_PUSHA("0x%p, %", map, key)
    if (map == NULL) TLFATAL("TLMap is NULL")
    if (key == NULL) TLFATAL("key is NULL")

    for (u16 i = 0; i < map->size; i++) {
        if (tl_string_equals(map->values[i].key, key)) {
            tl_memory_copy(
                map->values + i,
                map->values + i + 1,
                sizeof(struct TLMapEntry) * (map->size - map->length - 1)
            );

            map->length--;
            tl_memory_set(
                map->values + map->length,
                0,
                sizeof(struct TLMapEntry) * (map->size - map->length)
            );
        }
    }

    TL_STACK_POP
}

TLIterator* tl_map_keys(TLMap* map) {
    TL_STACK_PUSHA("0x%p", map)
    if (map == NULL) TLFATAL("TLMap is NULL")

    TLList *keys = tl_list_create(map->arena);
    if (map->length == 0) TL_STACK_POPV(NULL)

    for (u16 i = 0; i < map->length; ++i) {
        tl_list_add(keys, map->values[i].key);
    }

    TLIterator *it = tl_list_iterator_create(keys);
    TL_STACK_POPV(it)
}