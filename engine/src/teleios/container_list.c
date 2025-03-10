#include "teleios/container.h"
#include "teleios/memory.h"
#include "teleios/logger.h"

TLList* tl_list_create(void) {
    TLVERBOSE("tl_list_create(void)")
    TLList* list = tl_memory_alloc(sizeof(TLList), TL_MEMORY_CONTAINER_LIST);
    if (list == NULL) {
        TLERROR("Failed to allocate TLList")
        return NULL;
    }

    tl_memory_set(list, 0, sizeof(TLList));
    return list;
}

void tl_list_destroy(TLList* list) {
    TLVERBOSE("tl_list_destroy(0x%p)", list)
    if (list->length != 0) { TLWARN("Expected list->length == 0") }
    tl_memory_free((void*) list);
}