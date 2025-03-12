#include "teleios/container.h"
#include "teleios/memory.h"
#include "teleios/logger.h"

TLList* tl_list_create(TLMemoryArena* arena) {
    TLVERBOSE(">> tl_list_create(void)")
    TLList* list = tl_memory_alloc(arena, sizeof(TLList), TL_MEMORY_CONTAINER_LIST);
    if (list == NULL) {
        TLERROR("Failed to allocate TLList")
        TLVERBOSE("<< tl_list_create(void)")
        return NULL;
    }

    list->arena = arena;
    list->length = 0;

    TLVERBOSE("<< tl_list_create(void)")
    return list;
}