#ifndef __TELEIOS_CORE_CONTAINER_TYPES__
#define __TELEIOS_CORE_CONTAINER_TYPES__

#include "teleios/core/memory/types.h"

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

#endif //__TELEIOS_CORE_CONTAINER_TYPES__
