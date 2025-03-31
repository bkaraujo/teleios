#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"
// #####################################################################################################################
//
//                                                     MEMORY
//
// #####################################################################################################################
typedef struct {
    u64 index;
    char *payload;
} TLMemoryPage;

struct TLMemoryArena {
    u64 allocated;
    u64 tagged_count[TL_MEMORY_MAXIMUM];
    u64 tagged_size[TL_MEMORY_MAXIMUM];
    u64 page_size;
    TLMemoryPage page[U8_MAX];
};

static const char* tl_memory_name(const TLMemoryTag tag) {
    TLSTACKPUSHA("%d", tag)
    switch (tag) {
        case TL_MEMORY_BLOCK                : TLSTACKPOPV("TL_MEMORY_BLOCK")
        case TL_MEMORY_SERIALIZER           : TLSTACKPOPV("TL_MEMORY_SERIALIZER")
        case TL_MEMORY_CONTAINER_LIST       : TLSTACKPOPV("TL_MEMORY_CONTAINER_LIST")
        case TL_MEMORY_CONTAINER_STACK      : TLSTACKPOPV("TL_MEMORY_CONTAINER_STACK")
        case TL_MEMORY_CONTAINER_NODE       : TLSTACKPOPV("TL_MEMORY_CONTAINER_NODE")
        case TL_MEMORY_CONTAINER_ITERATOR   : TLSTACKPOPV("TL_MEMORY_CONTAINER_ITERATOR")
        case TL_MEMORY_STRING               : TLSTACKPOPV("TL_MEMORY_STRING")
        case TL_MEMORY_PROFILER             : TLSTACKPOPV("TL_MEMORY_PROFILER")
        case TL_MEMORY_SCENE                : TLSTACKPOPV("TL_MEMORY_SCENE")
        case TL_MEMORY_ULID                 : TLSTACKPOPV("TL_MEMORY_ULID")
        case TL_MEMORY_MAXIMUM              : TLSTACKPOPV("TL_MEMORY_MAXIMUM")
    }

    TLSTACKPOPV("???")
}

TLMemoryArena* tl_memory_arena_create(const u64 size) {
    TLSTACKPUSHA("%d", size)
    // ----------------------------------------------------------
    // Create the memory arena
    // ----------------------------------------------------------
    TLMemoryArena *arena = tl_platform_memory_alloc(sizeof(TLMemoryArena));
    if (arena == NULL) TLFATAL("Failed to allocate TLMemoryArena");
    tl_platform_memory_set(arena, 0, sizeof(TLMemoryArena));
    arena->page_size = size;
    // ----------------------------------------------------------
    // Keep track of the created arena
    // ----------------------------------------------------------
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->platform.memory.arenas[i] == NULL) {
            global->platform.memory.arenas[i] = arena;
            TLVERBOSE("TLMemoryArena 0x%p created with page size of %d bytes", arena, arena->page_size)
            TLSTACKPOPV(arena)
        }
    }

    TLFATAL("Failed to allocate TLMemoryArena");
}

TLINLINE static u8 tl_memory_arena_get_index(TLMemoryArena *arena) {
    TLSTACKPUSHA("0x%p", arena)

    if (arena == NULL) {
        TLWARN("TLMemoryArena is NULL")
        TLSTACKPOPV(FALSE)
    }

    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->platform.memory.arenas[i] == NULL) continue;
        if (global->platform.memory.arenas[i] == arena) {
            TLSTACKPOPV(i)
        }
    }

    TLFATAL("TLMemoryArena 0x%p not found", arena)
}

TLINLINE static void tl_memory_arena_do_destroy(const u8 index) {
    TLSTACKPUSHA("%d", index)
    TLMemoryArena *arena = global->platform.memory.arenas[index];
    for (u32 i = 0 ; i < TLARRSIZE(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload != NULL) {
            TLVERBOSE("TLMemoryArena 0x%p releasing page %d", arena, i)
            tl_platform_memory_free(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (global->platform.memory.arenas[index]->tagged_size[i] != 0) {
            TLVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    tl_platform_memory_free(arena);
    global->platform.memory.arenas[index] = NULL;

    TLSTACKPOP
}

void tl_memory_arena_reset(TLMemoryArena *arena) {
    TLSTACKPUSHA("0x%p", arena)
    for (u32 i = 0 ; i < TLARRSIZE(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload == NULL) break;

        arena->page[i].index = 0;
        tl_memory_set(arena->page[i].payload, 0, arena->page_size);
    }
    TLSTACKPOP
}

void tl_memory_arena_destroy(TLMemoryArena *arena) {
    TLSTACKPUSHA("0x%p", arena)
    const u8 index = tl_memory_arena_get_index(arena);
    tl_memory_arena_do_destroy(index);
    TLSTACKPOP
}

void *tl_memory_alloc(TLMemoryArena *arena, const u64 size, const TLMemoryTag tag) {
    TLSTACKPUSHA("0x%p, %llu, %d", arena, size, tag)
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (size == 0) {
        TLFATAL("TLMemoryArena 0x%p allocation size must be greater then 0", arena)
        TLSTACKPOPV(NULL)
    }

    if (size > arena->page_size) {
        TLFATAL("TLMemoryArena with page size of %d bytes. It cannot allocate %d bytes", arena, arena->page_size, size)
        TLSTACKPOPV(NULL)
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {

        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            TLVERBOSE("TLMemoryArena 0x%p initializing page %d", arena, i)
            arena->page[i].payload = tl_platform_memory_alloc(arena->page_size);
            tl_platform_memory_set(arena->page[i].payload, 0, arena->page_size);

            found = i;
            break;
        }

        // check if the page support the desired size
        if (arena->page[i].index + size <= arena->page_size) {
            found = i;
            break;
        }
    }

    if (found == U8_MAX) {
        TLWARN("TLMemoryArena 0x%p no suitable TLMemoryPage", arena)
        TLSTACKPOPV(NULL)
    }
    // -------------------------------------------------
    // Adjust the TLMemoryArena internal state
    // -------------------------------------------------
    arena->allocated += size;
    arena->tagged_count[tag] += 1;
    arena->tagged_size[tag] += size;
    // -------------------------------------------------
    // Adjust the TLMemoryPage internal state
    // -------------------------------------------------
    void* address = arena->page[found].payload + arena->page[found].index;
    TLVERBOSE("TLMemoryArena 0x%p page %d [remaning %llu] :: allocating %llu ", arena, found, arena->page_size - arena->page[found].index, size)
    arena->page[found].index += size;
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    TLSTACKPOPV(address)
}

void tl_memory_set(void *block, const i32 value, const u64 size) {
    TLSTACKPUSHA("0x%p, %d, %llu", block, value, size)
    tl_platform_memory_set(block, value, size);
    TLSTACKPOP
}

void tl_memory_copy(void *target, void *source, const u64 size) {
    TLSTACKPUSHA("0x%p, 0x%p, %llu", target, source, size)
    tl_platform_memory_copy(target, source, size);
    TLSTACKPOP
}
// #####################################################################################################################
//
//                                                     CONTAINER
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
    TLSTACKPUSHA("0x%p, 0X%p", arena, value)

    struct TLNode* created = tl_memory_alloc(arena, sizeof(struct TLNode), TL_MEMORY_CONTAINER_NODE);
    if (created == NULL) TLFATAL("Failed to allocate struct TLNode")
    created->payload = value;

    TLSTACKPOPV(created)
}

TLList* tl_list_create(TLMemoryArena *arena) {
    TLSTACKPUSHA("0x%p", arena)
    TLList* list = tl_memory_alloc(arena, sizeof(TLList), TL_MEMORY_CONTAINER_LIST);
    if (list == NULL) {
        TLERROR("Failed to allocate TLList")
        TLSTACKPOPV(NULL)
    }

    list->arena = arena;
    list->length = 0;

    TLSTACKPOPV(list)
}

u64 tl_list_length(TLList* list) {
    TLSTACKPUSHA("0x%p", list)
    TLSTACKPOPV(list->length)
}

void* tl_list_search(TLList* list, b8 (*PFN_filter)(void *value)) {
    TLSTACKPUSHA("0x%p, 0x%p", list, PFN_filter)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOPV(NULL)
    }

    if (PFN_filter == NULL) {
        TLERROR("PFN_filter is null")
        TLSTACKPOPV(NULL)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (PFN_filter(node->payload)) {
            TLSTACKPOPV(node->payload);
        }

        node = node->next;
    }

    TLSTACKPOPV(NULL)
}

void tl_list_foreach(TLList* list, void (*PFN_handler)(void *value)) {
    TLSTACKPUSHA("0x%p, 0x%p", list, PFN_handler)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOP
    }
    if (PFN_handler == NULL) {
        TLERROR("PFN_handler is NULL")
        TLSTACKPOP
    }

    const struct TLNode* node = list->head;
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
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOP
    }

    struct TLNode* created = tl_list_create_node(list->arena, value);

    if (list->head == NULL) {
        list->length++;
        list->head = created;
        list->tail = created;
        TLSTACKPOP
    }

    if (list->head == list->tail) {
        list->length++;
        list->tail = created;
        list->tail->previous = list->head;
        list->head->next = list->tail;
        TLSTACKPOP
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
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOP
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOP
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
    }

    if (list->tail->payload == value) {
        list->length--;
        list->tail->previous->next = NULL;
        list->tail = list->tail->previous;

        TLSTACKPOP
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

    TLSTACKPOP
}

b8 tl_list_after(TLList* list, void *item, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOPV(FALSE)
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOPV(FALSE)
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TLSTACKPOPV(FALSE)
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOPV(FALSE)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->arena, value);

            created->next = node->next;
            created->previous = node;
            node->next = created;
            TLSTACKPOPV(TRUE)
        }

        node = node->next;
    }

    TLSTACKPOPV(FALSE)
}

b8 tl_list_before(TLList* list, void *item, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p, 0x%p", list, item, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOPV(FALSE)
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOPV(FALSE)
    }

    if (item == NULL) {
        TLERROR("item is NULL")
        TLSTACKPOPV(FALSE)
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOPV(FALSE)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == item) {
            struct TLNode* created = tl_list_create_node(list->arena, value);

            created->next = node;
            created->previous = node->previous;
            node->previous->next = created;
            node->previous = created;
            TLSTACKPOPV(TRUE)
        }

        node = node->next;
    }

    TLSTACKPOPV(FALSE)
}

b8 tl_list_contains(TLList* list, void *value) {
    TLSTACKPUSHA("0x%p, 0x%p", list, value)
    if (list == NULL) {
        TLERROR("TLList is NULL")
        TLSTACKPOPV(FALSE)
    }

    if (list->head == NULL) {
        TLERROR("TLList is empty")
        TLSTACKPOPV(FALSE)
    }

    if (value == NULL) {
        TLERROR("value is NULL")
        TLSTACKPOPV(FALSE)
    }

    struct TLNode* node = list->head;
    while (node != NULL) {
        if (node->payload == value) {
            TLSTACKPOPV(TRUE)
        }

        node = node->next;
    }

    TLSTACKPOPV(FALSE)

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
    TLSTACKPOPV(iterator)
}

void* tl_list_iterator_next(TLIterator* iterator) {
    TLSTACKPUSHA("0x%p", iterator)
    if (iterator == NULL || iterator->node == NULL) {
        TLSTACKPOPV(NULL)
    }

    void* value = iterator->node->payload;
    iterator->node = iterator->node->next;
    TLSTACKPOPV(value)
}

TLStack* tl_stack_create(TLMemoryArena *arena)  {
    TLSTACKPUSHA("0x%p", arena)
    TLStack* stack = tl_memory_alloc(arena, sizeof(TLStack), TL_MEMORY_CONTAINER_STACK);
    stack = tl_list_create(arena);
    TLSTACKPOPV(stack)
}

void tl_stack_push(TLStack* stack, void* value) {
    TLSTACKPUSHA("0x%p, 0x%p", stack, value)
    tl_list_add(stack, value);
    TLSTACKPOP
}

void* tl_stack_peek(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    void* value = NULL;
    TLIterator* iterator = tl_list_iterator_create(stack);
    void* next = tl_list_iterator_next(iterator);
    while (next != NULL) { value = next; next = tl_list_iterator_next(iterator); }
    TLSTACKPOPV(value)
}

void* tl_stack_pop(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    void* value = tl_stack_peek(stack);
    tl_list_remove(stack, value);
    TLSTACKPOPV(value)
}

u64 tl_stack_length(TLStack* stack) {
    TLSTACKPUSHA("0x%p", stack)
    TLSTACKPOPV(tl_list_length(stack))
}

// #####################################################################################################################
//
//                                                     INPUT
//
// #####################################################################################################################

b8 tl_input_is_key_active(const i32 key) {
    TLSTACKPUSHA("%d", key)
    const b8 result = global->platform.input.keyboard.key[key];
    TLSTACKPOPV(result)
}

b8 tl_input_is_key_pressed(const i32 key) {
    TLSTACKPUSHA("%d", key)
    TLINFO("????")
    const b8 frame_previous = global->application.frame.last.input.keyboard.key[key];
    const b8 frame_current = global->application.frame.current.input.keyboard.key[key];
    TLSTACKPOPV(!frame_previous && frame_current)
}

b8 tl_input_is_key_released(const i32 key) {
    TLSTACKPUSHA("%d", key)
    const b8 frame_previous = global->application.frame.last.input.keyboard.key[key];
    const b8 frame_current = global->application.frame.current.input.keyboard.key[key];
    TLSTACKPOPV(frame_previous && !frame_current)
}
// #####################################################################################################################
//
//                                                     STRING
//
// #####################################################################################################################
struct TLString {
    u64 size;
    u64 length;
    const char *text;
    TLMemoryArena *arena;
    b8 is_view;
};

TLINLINE static TLString* tl_string_reserve(TLMemoryArena *arena, const u64 size) {
    TLSTACKPUSHA("0x%p, %d", arena, size)
    TLString *string = tl_memory_alloc(arena, sizeof(struct TLString), TL_MEMORY_STRING);
    string->is_view = FALSE;
    string->arena = arena;
    string->length = 0;
    if (size > 0) {
        string->size = size + 1; // extra byte for NULL character
        string->text = tl_memory_alloc(arena, string->size, TL_MEMORY_STRING);
    }

    TLSTACKPOPV(string)
}

TLString* tl_string_clone(TLMemoryArena *arena, const char *string) {
    TLSTACKPUSHA("0x%p, 0x%p", arena, string)
    TLString *clone = tl_string_reserve(arena, tl_char_length(string));
    clone->length = clone->size - 1;
    tl_memory_copy((void*)clone->text, (void*)string, clone->length);
    TLSTACKPOPV(clone)
}

TLString* tl_string_wrap(TLMemoryArena *arena, const char *string) {
    TLSTACKPUSHA("0x%p, 0x%p", arena, string)
    TLString *wrap = tl_string_reserve(arena, 0);
    wrap->is_view = TRUE;
    wrap->text = string;
    wrap->size = wrap->length;
    wrap->length = tl_char_length(string);
    TLSTACKPOPV(wrap)
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
TLString* tl_string_from_i32(TLMemoryArena *arena, i32 value, const u8 base) {
    TLSTACKPUSHA("%d, %d", value, base)

    u32 digits = 0;
    i32 desired = value;
    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);

    TLString* string = tl_string_reserve(arena, digits);

    // check that the base if valid
    if (base < 2 || base > 36) { TLSTACKPOPV(string) }

    int tmp_value;
    char *ptr  = (char*) string->text;
    char *ptr1 = (char*) string->text;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        string->length++;
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';

    // Reverse the string
    while(ptr1 < ptr) {
        const char tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
        string->length++;
    }

    TLSTACKPOPV(string)
}

void tl_string_join(const TLString *string, const char *other) {
    TLSTACKPUSHA("0x%p, 0x%p", string, other)
    const u64 length = tl_char_length(other);
    if (length == U32_MAX || length == 0) TLFATAL("Failed to join:\n\n - 0%xs\n - %s", string->text, other)
    ((TLString*)string)->size = string->size + length;

    void *created = tl_memory_alloc(string->arena, string->size, TL_MEMORY_STRING);
    tl_memory_copy(created, (void*)string->text, string->length);
    tl_memory_copy(created + string->length, (void*)other, length);

    ((TLString*)string)->text = created;
    ((TLString*)string)->length = string->size - 1;
    ((TLString*)string)->is_view = FALSE;

    TLSTACKPOP
}

TLINLINE const char * tl_string(TLString *string) {
    TLSTACKPUSHA("0x%p", string)
    TLSTACKPOPV(string->text)
}

TLString* tl_string_slice(TLMemoryArena *arena, TLString* string, const u64 offset, const u64 length) {
    TLSTACKPUSHA("0x%p, 0x%p, %d, %d", arena, string, offset, length)
    //TODO implement tl_string_slice
    TLFATAL("Implementation missing")
    TLSTACKPOPV(NULL)
}

TLString* tl_string_view(TLString* string) {
    TLSTACKPUSHA("0x%p", string)
    TLString *view = tl_string_reserve(string->arena, 0);
    view->is_view = TRUE;
    view->text = string->text;
    view->size = string->size;
    view->length = string->length;
    TLSTACKPOPV(view)
}

u32 tl_string_length(TLString *string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL) {
        TLSTACKPOPV(-1)
    }
    TLSTACKPOPV(string->length)
}

u32 tl_string_index_of(TLString* string, const char token) {
    TLSTACKPUSHA("0x%p, %s", string, token)
    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] == token) TLSTACKPOPV(i);
    }
    TLSTACKPOPV(U32_MAX)
}

u32 tl_string_last_index_of(TLString* string, const char token) {
    TLSTACKPUSHA("0x%p, %s", string, token)
    if (string == NULL) TLSTACKPOPV(U32_MAX);

    if (string->length == 0) {
        if (string->text[0] == token) TLSTACKPOPV(0);
        TLSTACKPOPV(U32_MAX)
    }

    for (u64 i = string->length - 1; i > 0; --i) {
        if (string->text[i] == token) TLSTACKPOPV(i);
    }
    TLFATAL("Implementation missing")
    TLSTACKPOPV(FALSE)
}

b8 tl_string_start_with(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) TLSTACKPOPV(FALSE)

    const u64 length = tl_char_length(guess);
    if (length > string->length) TLSTACKPOPV(FALSE)

    for (u64 i = 0; i < length; ++i) {
        if (string->text[i] != guess[i])
            TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_string_ends_with(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_ends_with
    TLFATAL("Implementation missing")
    TLSTACKPOPV(FALSE)
}

b8 tl_string_is_view(const TLString* string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL) {
        TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(string->is_view)
}

b8 tl_string_equals(const TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOPV(FALSE)
    }

    const u64 length = tl_char_length(guess);
    if (string->length != length) TLSTACKPOPV(FALSE)

    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] != guess[i])
            TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_string_contains(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_contains
    TLFATAL("Implementation missing")
    TLSTACKPOPV(FALSE)
}

// #####################################################################################################################
//
//                                                     SERIALIZER
//
// #####################################################################################################################
#include <ctype.h>
#include <stdio.h>

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) TLFATAL("Unexpect token")

typedef struct {
    const TLString *name;
    u32 sequence;
} TLTuple;

void tl_serializer_walk(void (*processor)(const char* prefix, const char* block, const char *value)) {
    TLSTACKPUSHA("%s", tl_string(global->yaml))
    FILE* file = fopen(tl_string(global->yaml), "r");
    if (file == NULL) TLFATAL("Failed to open %s", tl_string(global->yaml));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); TLFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    u8 block_index = 0;
    u16 prefix_index = 0;
    yaml_token_t token;
    TLMemoryArena *arena = tl_memory_arena_create(TLKIBIBYTES(4));
    TLList *sequences = tl_list_create(arena);
    char block[U8_MAX]; // current YAML_KEY_TOKEN
    char prefix[U16_MAX]; // YAML_KEY_TOKEN path to YAML_SCALAR_TOKEN value

    do {
        yaml_parser_scan(&parser, &token);

        switch(token.type) {
            default: continue;
            case YAML_NO_TOKEN: {
                TLFATAL("Malformed YAML token");
            } break;
            // #########################################################################################################
            // YAML_BLOCK_SEQUENCE_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_SEQUENCE_START_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                TLTuple *tuple = tl_memory_alloc(arena, sizeof(TLTuple), TL_MEMORY_SERIALIZER);
                tuple->sequence = 0;

                tuple->name = tl_string_clone(arena, prefix);
                tl_string_join(tuple->name, block);
                tl_string_join(tuple->name, ".");

                tl_list_add(sequences, tuple);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_MAPPING_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_MAPPING_START_TOKEN: {
                EXPECT(YAML_KEY_TOKEN)
                EXPECT(YAML_SCALAR_TOKEN)
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                if (block_index != 0) {
                    tl_memory_copy(prefix + prefix_index, block, block_index);
                    prefix_index += block_index;

                    tl_memory_copy(prefix + prefix_index, ".", 1);
                    prefix_index++;
                }
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                tl_memory_copy(block, token.data.scalar.value, token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(block + token.data.scalar.length, 0, block_index - token.data.scalar.length);
                }
                block_index = token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_BLOCK_ENTRY_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_ENTRY_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                if (block_index != 0) {
                    tl_memory_copy(prefix + prefix_index, block, block_index);
                    prefix_index += block_index;

                    tl_memory_copy(prefix + prefix_index, ".", 1);
                    prefix_index++;
                }
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                TLString *string = NULL;

                // -----------------------------------------------------------
                // Try tro increase the counter
                // -----------------------------------------------------------
                TLIterator *iterator = tl_list_iterator_create(sequences);
                for (TLTuple *tuple = tl_list_iterator_next(iterator) ; tuple != NULL; tuple = tl_list_iterator_next(iterator)) {
                    if (tl_string_equals(tuple->name, prefix)) {
                        string = tl_string_from_i32(arena, (i32) tuple->sequence, 10);
                        tuple->sequence++;
                    }
                }

                u32 length = tl_string_length(string);
                tl_memory_copy(block, (void*) tl_string(string), length);

                if (block_index > length) {
                    tl_memory_set(block + length, 0, block_index - length);
                }

                block_index = tl_string_length(string);
            } continue;
            // #########################################################################################################
            // YAML_KEY_TOKEN
            // #########################################################################################################
            case YAML_KEY_TOKEN: {
                EXPECT(YAML_SCALAR_TOKEN)
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                tl_memory_copy(block, token.data.scalar.value, token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(block + token.data.scalar.length, 0, block_index - token.data.scalar.length);
                }

                block_index = token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_SCALAR_TOKEN
            // #########################################################################################################
            case YAML_SCALAR_TOKEN: {
                processor(prefix, block, (const char*)token.data.scalar.value);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_END_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_END_TOKEN: {
                b8 found = FALSE;
                for (u16 i = prefix_index - 2 ; i > 0 ; --i) {
                    if (*(prefix + i)  == '.') {
                        tl_memory_set(prefix + i + 1, 0, U16_MAX - i);
                        prefix_index = i + 1;
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    tl_memory_set(prefix, 0, U16_MAX);
                    prefix_index = 0;
                }

                tl_memory_set(block, 0, U8_MAX - block_index);
                block_index = 0;
            } break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    tl_memory_arena_destroy(arena);

    TLSTACKPOP
}
// #####################################################################################################################
//
//                                                     SCRIPT
//
// #####################################################################################################################
#define ERROR(s) TLSTACKPOPV(luaL_error(state, s))

i32 tl_script_api__is_key_pressed(lua_State *state) {
    TLSTACKPUSHA("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 1) ERROR("Expected a single value: isKeyPressed(KEY)")
    if (!lua_isnumber(state, 1)) ERROR("parameter [KEY] must be a valid key")
    // =========================================================================
    // Request execution
    // =========================================================================
    const i32 key = lua_tonumber(state, 1);
    lua_pushnumber(state, tl_input_is_key_pressed(key));
    // =========================================================================
    // LUA stack push
    // =========================================================================
    TLSTACKPOPV(1)
}

i32 tl_script_api__is_key_released(lua_State *state) {
    TLSTACKPUSHA("0x%p", state)
    // =========================================================================
    // Parameters validation
    // =========================================================================
    if (lua_gettop(state) != 1) ERROR("Expected a single value: isKeyReleased(KEY)")
    if (!lua_isnumber(state, 1)) ERROR("parameter [KEY] must be a valid key")
    // =========================================================================
    // Request execution
    // =========================================================================
    const i32 key = lua_tonumber(state, 1);
    lua_pushnumber(state, tl_input_is_key_released(key));
    // =========================================================================
    // LUA stack push
    // =========================================================================
    TLSTACKPOPV(1)
}

b8 tl_script_initialize(void) {
    TLSTACKPUSH

    TLDEBUG("LUA_VERSION %s.%s.%s", LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE);
    global->platform.script.state = luaL_newstate();
    if (global->platform.script.state == NULL) {
        TLERROR("Failed to initialize LUA")
    }

    luaL_openlibs(global->platform.script.state);

    // Tabela para registrar as funções
    static const luaL_Reg functions[] = {
        {"isKeyPressed", tl_script_api__is_key_pressed},
        {"isKeyReleased", tl_script_api__is_key_released},
        {NULL, NULL}
    };

    luaL_newlib(global->platform.script.state, functions);
    lua_setglobal(global->platform.script.state, "teleios_input");

    {
        if (luaL_dofile(global->platform.script.state, "/mnt/nvme1/Cloud/Google/Trabalho/bkraujo/teleios/sandbox/assets/scripts/environment.lua") != LUA_OK) {
            fprintf(stderr, "Erro ao executar arquivo: %s\n", lua_tostring(global->platform.script.state, -1));
        }
    }


    TLSTACKPOPV(TRUE)
}

b8 tl_script_terminate(void) {
    TLSTACKPUSH
    lua_close(global->platform.script.state);
    global->platform.script.state = NULL;
    TLSTACKPOPV(TRUE)
}
// #####################################################################################################################
//
//                                                     ULID
//
// #####################################################################################################################
#ifdef TLPLATFORM_WINDOWS
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   pragma comment(lib, "advapi32.lib")
#elif TLPLATFORM_LINUX
// #   define _GNU_SOURCE
#   include <sys/time.h>
#   include <sys/syscall.h>
#elif TLPLATFORM_POSIX
#   define _POSIX_C_SOURCE 200112L
#   include <sys/time.h>
#else
#   error "!!!"
#endif

#include <time.h>

struct TLUlidGenerator {
    u8 last[16];
    u64 timestamp;
    i32 flags;
    u8 i, j;
    u8 s[256];
};

struct TLUlid {
    char text[27];
};

static int platform_entropy(void *buf, const i32 len) {
    TLSTACKPUSHA("0x%p, %d", buf, len)
#if _WIN32
    BOOLEAN NTAPI SystemFunction036(PVOID, ULONG);
    TLSTACKPOPV(!SystemFunction036(buf, len))
#elif __linux__
    i32 result = syscall(SYS_getrandom, buf, len, 0);
    TLSTACKPOPV(result != len)
#else
    int r = 0;
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        r = fread(buf, len, 1, f);
        fclose(f);
    }
    TLSTACKPOPV(!r)
#endif
}

TLUlidGenerator* tl_ulid_generator_init(TLMemoryArena *arena, const i32 flags) {
    TLSTACKPUSHA("0x%p, %d", arena, flags)
    TLUlidGenerator* generator = tl_memory_alloc(arena, sizeof(TLUlidGenerator), TL_MEMORY_ULID);

    generator->timestamp = 0;
    generator->flags = flags;
    generator->i = generator->j = 0;
    for (i32 i = 0; i < 256; i++)
        generator->s[i] = i;

    /* RC4 is used to fill the random segment of ULIDs. It's tiny,
     * simple, perfectly sufficient for the task (assuming it's seeded
     * properly), and doesn't require fixed-width integers. It's not the
     * fastest option, but it's plenty fast for the task.
     *
     * Besides, when we're in a serious hurry in normal operation (not
     * in "relaxed" mode), we're incrementing the random field much more
     * often than generating fresh random bytes.
     */

    u8 key[256] = {0};
    if (!platform_entropy(key, 256)) {
        /* Mix entropy into the RC4 state. */
        for (i32 i = 0, j = 0; i < 256; ++i) {
            j = (j + generator->s[i] + key[i]) & 0xff;

            const i32 tmp = generator->s[i];
            generator->s[i] = generator->s[j];
            generator->s[j] = tmp;
        }

        TLSTACKPOPV(generator)
    }

    if (!(flags & ULID_SECURE)) {
        /* Failed to read entropy from OS, so generate some. */
        u64 now, n = 0;
        const u64 start = tl_time_epoch_millis();

        do {
            struct {
                clock_t clk;
                u64 ts;
                i64 n;
                void *stackgap;
            } noise;

            noise.ts = now = tl_time_epoch_millis();
            noise.clk = clock();
            noise.stackgap = &noise;
            noise.n = (i64) n;

            const u8 *k = (u8 *)&noise;
            for (i32 i = 0, j = 0; i < 256; ++i) {
                j = (j + generator->s[i] + k[i % sizeof(noise)]) & 0xff;
                const i32 tmp = generator->s[i];
                generator->s[i] = generator->s[j];
                generator->s[j] = tmp;
            }
        } while (n++ < 1UL << 16 || now - start < 500000ULL);
        TLSTACKPOPV(generator)
    }

    TLSTACKPOPV(NULL)
}

void tl_ulid_encode(TLUlid* ulid, const u8 binary[16]) {
    static const i8 set[256] = {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
        0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x50, 0x51,
        0x52, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a
    };

    ulid->text[ 0] = set[ binary[ 0] >> 5];
    ulid->text[ 1] = set[ binary[ 0] >> 0];
    ulid->text[ 2] = set[ binary[ 1] >> 3];
    ulid->text[ 3] = set[(binary[ 1] << 2 | binary[ 2] >> 6) & 0x1f];
    ulid->text[ 4] = set[ binary[ 2] >> 1];
    ulid->text[ 5] = set[(binary[ 2] << 4 | binary[ 3] >> 4) & 0x1f];
    ulid->text[ 6] = set[(binary[ 3] << 1 | binary[ 4] >> 7) & 0x1f];
    ulid->text[ 7] = set[ binary[ 4] >> 2];
    ulid->text[ 8] = set[(binary[ 4] << 3 | binary[ 5] >> 5) & 0x1f];
    ulid->text[ 9] = set[ binary[ 5] >> 0];
    ulid->text[10] = set[ binary[ 6] >> 3];
    ulid->text[11] = set[(binary[ 6] << 2 | binary[ 7] >> 6) & 0x1f];
    ulid->text[12] = set[ binary[ 7] >> 1];
    ulid->text[13] = set[(binary[ 7] << 4 | binary[ 8] >> 4) & 0x1f];
    ulid->text[14] = set[(binary[ 8] << 1 | binary[ 9] >> 7) & 0x1f];
    ulid->text[15] = set[ binary[ 9] >> 2];
    ulid->text[16] = set[(binary[ 9] << 3 | binary[10] >> 5) & 0x1f];
    ulid->text[17] = set[ binary[10] >> 0];
    ulid->text[18] = set[ binary[11] >> 3];
    ulid->text[19] = set[(binary[11] << 2 | binary[12] >> 6) & 0x1f];
    ulid->text[20] = set[ binary[12] >> 1];
    ulid->text[21] = set[(binary[12] << 4 | binary[13] >> 4) & 0x1f];
    ulid->text[22] = set[(binary[13] << 1 | binary[14] >> 7) & 0x1f];
    ulid->text[23] = set[ binary[14] >> 2];
    ulid->text[24] = set[(binary[14] << 3 | binary[15] >> 5) & 0x1f];
    ulid->text[25] = set[ binary[15] >> 0];
    ulid->text[26] = 0;
}

// static b8 ulid_decode(u8 ulid[16], const i8 *s) {
//     TLSTACKPUSHA("0x%p, 0x%p", ulid, s)
//     static const i8 v[] = {
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//         0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
//         0x08, 0x09,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
//         0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
//         0x16, 0x17, 0x18, 0x19, 0x1a,   -1, 0x1b, 0x1c,
//         0x1d, 0x1e, 0x1f,   -1,   -1,   -1,   -1,   -1,
//           -1, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
//         0x11, 0x01, 0x12, 0x13, 0x01, 0x14, 0x15, 0x00,
//         0x16, 0x17, 0x18, 0x19, 0x1a,   -1, 0x1b, 0x1c,
//         0x1d, 0x1e, 0x1f,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
//           -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
//     };
//     if (v[(i32)s[0]] > 7)
//         TLSTACKPOPV(TRUE)
//
//     for (i32 i = 0; i < 26; i++)
//         if (v[(i32)s[i]] == -1)
//             TLSTACKPOPV(TRUE)
//
//     ulid[ 0] = v[(i32)s[ 0]] << 5 | v[(i32)s[ 1]] >> 0;
//     ulid[ 1] = v[(i32)s[ 2]] << 3 | v[(i32)s[ 3]] >> 2;
//     ulid[ 2] = v[(i32)s[ 3]] << 6 | v[(i32)s[ 4]] << 1 | v[(i32)s[ 5]] >> 4;
//     ulid[ 3] = v[(i32)s[ 5]] << 4 | v[(i32)s[ 6]] >> 1;
//     ulid[ 4] = v[(i32)s[ 6]] << 7 | v[(i32)s[ 7]] << 2 | v[(i32)s[ 8]] >> 3;
//     ulid[ 5] = v[(i32)s[ 8]] << 5 | v[(i32)s[ 9]] >> 0;
//     ulid[ 6] = v[(i32)s[10]] << 3 | v[(i32)s[11]] >> 2;
//     ulid[ 7] = v[(i32)s[11]] << 6 | v[(i32)s[12]] << 1 | v[(i32)s[13]] >> 4;
//     ulid[ 8] = v[(i32)s[13]] << 4 | v[(i32)s[14]] >> 1;
//     ulid[ 9] = v[(i32)s[14]] << 7 | v[(i32)s[15]] << 2 | v[(i32)s[16]] >> 3;
//     ulid[10] = v[(i32)s[16]] << 5 | v[(i32)s[17]] >> 0;
//     ulid[11] = v[(i32)s[18]] << 3 | v[(i32)s[19]] >> 2;
//     ulid[12] = v[(i32)s[19]] << 6 | v[(i32)s[20]] << 1 | v[(i32)s[21]] >> 4;
//     ulid[13] = v[(i32)s[21]] << 4 | v[(i32)s[22]] >> 1;
//     ulid[14] = v[(i32)s[22]] << 7 | v[(i32)s[23]] << 2 | v[(i32)s[24]] >> 3;
//     ulid[15] = v[(i32)s[24]] << 5 | v[(i32)s[25]] >> 0;
//
//     TLSTACKPOPV(FALSE)
// }

TLUlid* tl_ulid_generate(TLMemoryArena *arena, TLUlidGenerator *generator) {
    TLSTACKPUSHA("0x%p, 0x%p", arena, generator)
    const u64 timestamp = tl_time_epoch_micros() / 1000;
    TLUlid* ulid = tl_memory_alloc(arena, sizeof(TLUlid), TL_MEMORY_ULID);

    if (!(generator->flags & ULID_RELAXED) && generator->timestamp == timestamp) {
        /* Chance of 80-bit overflow is so small that it's not considered. */
        for (i32 i = 15; i > 5; i--)
            if (++generator->last[i])
                break;

        tl_ulid_encode(ulid, generator->last);
        TLSTACKPOPV(ulid)
    }

    /* Fill out timestamp */
    generator->timestamp = timestamp;
    generator->last[0] = timestamp >> 40;
    generator->last[1] = timestamp >> 32;
    generator->last[2] = timestamp >> 24;
    generator->last[3] = timestamp >> 16;
    generator->last[4] = timestamp >>  8;
    generator->last[5] = timestamp >>  0;

    /* Fill out random section */
    for (i32 k = 0; k < 10; k++) {
        generator->i = (generator->i + 1) & 0xff;
        generator->j = (generator->j + generator->s[generator->i]) & 0xff;

        const i32 tmp = generator->s[generator->i];
        generator->s[generator->i] = generator->s[generator->j];
        generator->s[generator->j] = tmp;
        generator->last[6 + k] = generator->s[(generator->s[generator->i] + generator->s[generator->j]) & 0xff];
    }

    if (generator->flags & ULID_PARANOID)
        generator->last[6] &= 0x7f;

    tl_ulid_encode(ulid, generator->last);
    TLSTACKPOPV(ulid)
}

TLString* tl_ulid(TLMemoryArena *arena, TLUlid * ulid) {
    TLSTACKPUSHA("0x%p, 0x%p", arena, ulid)
    TLString* string = tl_string_clone(arena, ulid->text);
    TLSTACKPOPV(string)
}
// #####################################################################################################################
//
//                                                     SERIALIZER
//
// #####################################################################################################################
static void tl_serializer_read(const char *prefix, const char *element, const char *value) {
    TLSTACKPUSHA("%s, %s, %s", prefix, element, value)

    if (tl_char_start_with(prefix, "application.")) {
        if (tl_char_start_with(prefix, "application.scenes.")) TLSTACKPOP

        if (tl_char_equals(prefix, "application.")) {
            if (tl_char_equals(element, "version")) {
                global->application.version = tl_string_clone(global->platform.arena, value);
                TLTRACE("global->application.version = %s", value)
                TLSTACKPOP
            }
        }

        TLSTACKPOP
    }

    if (tl_char_start_with(prefix, "engine.")) {
        if (tl_char_equals(prefix, "engine.logging.") && tl_char_equals(element, "level")) {
            if (tl_char_equals(value, "verbose")) { tl_logger_loglevel(TL_LOG_LEVEL_VERBOSE); TLSTACKPOP }
            if (tl_char_equals(value,   "trace")) { tl_logger_loglevel(  TL_LOG_LEVEL_TRACE); TLSTACKPOP }
            if (tl_char_equals(value,   "debug")) { tl_logger_loglevel(  TL_LOG_LEVEL_DEBUG); TLSTACKPOP }
            if (tl_char_equals(value,    "info")) { tl_logger_loglevel(   TL_LOG_LEVEL_INFO); TLSTACKPOP }
            if (tl_char_equals(value,    "warn")) { tl_logger_loglevel(   TL_LOG_LEVEL_WARN); TLSTACKPOP }
            if (tl_char_equals(value,   "error")) { tl_logger_loglevel(  TL_LOG_LEVEL_ERROR); TLSTACKPOP }
            if (tl_char_equals(value,   "fatal")) { tl_logger_loglevel(  TL_LOG_LEVEL_FATAL); TLSTACKPOP }
        }

        if (tl_char_equals(prefix, "engine.graphics.")) {
            if (tl_char_equals(element, "vsync")) {
                global->platform.graphics.vsync = tl_char_equals(value, "true");
                TLTRACE("global->platform.graphics.vsync = %d", global->platform.graphics.vsync)
                TLSTACKPOP
            }
            if (tl_char_equals(element, "wireframe")) {
                global->platform.graphics.wireframe = tl_char_equals(value, "true");
                TLTRACE("global->platform.graphics.wireframe = %d", global->platform.graphics.wireframe)
                TLSTACKPOP
            }
        }

        if (tl_char_equals(prefix, "engine.simulation.") && tl_char_equals(element, "step")) {
            u8 step = strtol(value, (void*)(value + tl_char_length(value)), 10);
            if (step == 0) {
                TLWARN("Failed to read [%s%s] assuming 24", prefix, value);
                step = 24;
            }

            global->application.simulation.step = 1.0f / (f64) step;
            TLTRACE("global->simulation.step = %f", global->application.simulation.step)
            TLSTACKPOP
        }

        if (tl_char_equals(prefix, "engine.window.")) {
            if (tl_char_equals(element, "title")) {
                global->platform.window.title = tl_string_clone(global->platform.arena, value);
                TLTRACE("global->platform.window.title = %s", value)
                TLSTACKPOP
            }

            if (tl_char_equals(element, "size")) {
                if (tl_char_equals(value,  "SD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_SD; }
                if (tl_char_equals(value,  "HD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_HD; }
                if (tl_char_equals(value, "FHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_FHD; }
                if (tl_char_equals(value, "QHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_QHD; }
                if (tl_char_equals(value, "UHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_UHD; }

                global->platform.window.size.y = (global->platform.window.size.x * 9) / 16;
                TLTRACE("global->platform.window.size = %u x %u", global->platform.window.size.x, global->platform.window.size.y)
                TLSTACKPOP
            }
        }

        TLSTACKPOP
    }

    TLWARN("Unknown prefix: %s", prefix)
    TLSTACKPOP
}

static void tl_window_callback_window_closed(GLFWwindow* window) {
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
}

static void tl_window_callback_window_pos(GLFWwindow* _, const int xPos, const int yPos) {
    global->platform.window.position.x = xPos;
    global->platform.window.position.y = yPos;

    TLEvent event = { 0 };
    event.i32[0] = global->platform.window.position.x;
    event.i32[1] = global->platform.window.position.y;

    tl_event_submit(TL_EVENT_WINDOW_MOVED, &event);
}

static void tl_window_callback_window_size(GLFWwindow* window, const int width, const int height) {
    global->platform.window.size.x = width;
    global->platform.window.size.y = height;

    TLEvent event = { 0 };
    event.i32[0] = global->platform.window.size.x;
    event.i32[1] = global->platform.window.size.y;

    tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);
}

static void tl_window_callback_window_focus(GLFWwindow* window, const i32 focused) {
    global->platform.window.focused = focused;

    tl_event_submit(focused ? TL_EVENT_WINDOW_FOCUS_GAINED : TL_EVENT_WINDOW_FOCUS_LOST, NULL);
}

static void tl_window_callback_window_minimized(GLFWwindow* window, const i32 minimized) {
    global->platform.window.maximized = FALSE;
    global->platform.window.minimized = minimized;

    tl_event_submit(minimized ? TL_EVENT_WINDOW_MINIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_window_maximize(GLFWwindow* window, const i32 maximized) {
    global->platform.window.maximized = maximized;
    global->platform.window.minimized = FALSE;

    tl_event_submit(maximized ? TL_EVENT_WINDOW_MAXIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_input_keyboard(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
    i32 type;

    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_KEY_PRESSED;
        global->platform.input.keyboard.key[key] = TRUE;
    } else {
        type = TL_EVENT_INPUT_KEY_RELEASED;
        global->platform.input.keyboard.key[key] = FALSE;
    }

    TLEvent event = { 0 };
    event.u32[0] = key;
    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_position(GLFWwindow* window, const double xpos, const double ypos) {
    global->platform.input.cursor.position_x = (u32) xpos;
    global->platform.input.cursor.position_y = (u32) ypos;

    TLEvent event = { 0 };
    event.u32[0] = global->platform.input.cursor.position_x;
    event.u32[1] = global->platform.input.cursor.position_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_MOVED, &event);
}

static void tl_window_callback_input_cursor_button(GLFWwindow* window, const int button, const int action, const int mods) {
    i32 type;

    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_CURSOR_PRESSED;
        global->platform.input.cursor.button[button] = TRUE;
    } else {
        type = TL_EVENT_INPUT_CURSOR_RELEASED;
        global->platform.input.cursor.button[button] = FALSE;
    }

    global->platform.input.cursor.button[button] = action == GLFW_PRESS;

    TLEvent event = { 0 };
    event.u32[0] = button;

    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_scroll(GLFWwindow* window, const double xoffset, const double yoffset) {
    global->platform.input.cursor.scroll_x = xoffset > 0 ? 1 : xoffset < 0 ? -1 : 0;
    global->platform.input.cursor.scroll_y = yoffset > 0 ? 1 : yoffset < 0 ? -1 : 0;

    TLEvent event = { 0 };
    event.u8[0] = global->platform.input.cursor.scroll_x;
    event.u8[1] = global->platform.input.cursor.scroll_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_SCROLLED, &event);
}

static void tl_window_callback_input_cursor_entered(GLFWwindow* window, const int entered) {
    global->platform.input.cursor.hoover = entered == GLFW_TRUE;
    tl_event_submit(entered ? TL_EVENT_INPUT_CURSOR_ENTERED : TL_EVENT_INPUT_CURSOR_EXITED, NULL);
}

static b8 tl_window_create(void) {
    TLSTACKPUSH

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // --------------------------------------------------------------------------------------
    // Disable window framebuffer bits we don't need, because we render into offscreen FBO and blit to window.
    // --------------------------------------------------------------------------------------
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);
    // --------------------------------------------------------------------------------------
    // Create the platform surface
    // --------------------------------------------------------------------------------------
    TLDEBUG(
        "Window (%u x %u) :: %s",
        global->platform.window.size.x,
        global->platform.window.size.y,
        tl_string(global->platform.window.title)
    )

    global->platform.window.handle = glfwCreateWindow(
        global->platform.window.size.x,
        global->platform.window.size.y,
        tl_string(global->platform.window.title),
        NULL, NULL
    );

    if (global->platform.window.handle == NULL) {
        TLERROR("Failed to create GLFW window");
        TLSTACKPOPV(FALSE)
    }
    // --------------------------------------------------------------------------------------
    // Cache state
    // --------------------------------------------------------------------------------------
    global->platform.window.visible = FALSE;
    global->platform.window.focused = glfwGetWindowAttrib(global->platform.window.handle, GLFW_FOCUSED) == GLFW_TRUE;
    global->platform.window.maximized = glfwGetWindowAttrib(global->platform.window.handle, GLFW_MAXIMIZED) == GLFW_TRUE;
    global->platform.window.minimized = glfwGetWindowAttrib(global->platform.window.handle, GLFW_ICONIFIED) == GLFW_TRUE;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (mode != NULL) {
        global->platform.window.position.x = (mode->width - global->platform.window.size.x) / 2;
        global->platform.window.position.y = (mode->height - global->platform.window.size.y) / 2;
    }

    glfwSetWindowPos(global->platform.window.handle, global->platform.window.position.x, global->platform.window.position.y);
    // --------------------------------------------------------------------------------------
    // Window callbacks
    // --------------------------------------------------------------------------------------
    glfwSetWindowCloseCallback      (global->platform.window.handle, tl_window_callback_window_closed);
    glfwSetWindowPosCallback        (global->platform.window.handle, tl_window_callback_window_pos);
    glfwSetWindowSizeCallback       (global->platform.window.handle, tl_window_callback_window_size);
    glfwSetWindowFocusCallback      (global->platform.window.handle, tl_window_callback_window_focus);
    glfwSetWindowIconifyCallback    (global->platform.window.handle, tl_window_callback_window_minimized);
    glfwSetWindowMaximizeCallback   (global->platform.window.handle, tl_window_callback_window_maximize);
    // --------------------------------------------------------------------------------------
    // Input callbacks
    // --------------------------------------------------------------------------------------
    glfwSetKeyCallback              (global->platform.window.handle, tl_window_callback_input_keyboard);
    glfwSetMouseButtonCallback      (global->platform.window.handle, tl_window_callback_input_cursor_button);
    glfwSetCursorPosCallback        (global->platform.window.handle, tl_window_callback_input_cursor_position);
    glfwSetScrollCallback           (global->platform.window.handle, tl_window_callback_input_cursor_scroll);
    glfwSetCursorEnterCallback      (global->platform.window.handle, tl_window_callback_input_cursor_entered);

    TLSTACKPOPV(TRUE)
}

b8 tl_runtime_initialize(void) {
    TLSTACKPUSH

    tl_serializer_walk(tl_serializer_read);

    if (!tl_window_create()) {
        TLERROR("Failed to create application window");
        TLSTACKPOPV(FALSE)
    }

    if (!tl_script_initialize()) {
        TLERROR("Failed to initialize script engine");
        TLSTACKPOPV(FALSE)
    }

    if (!tl_graphics_initialize()) {
        TLERROR("Failed to initialize Graphics API")
        TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_runtime_terminate(void) {
    if (!tl_script_terminate()) return FALSE;
    if (!tl_graphics_terminate()) TLERROR("Failed to terminate graphics engine");

    return TRUE;
}