#ifndef __TELEIOS_MEMORY_TYPES__
#define __TELEIOS_MEMORY_TYPES__

#include "teleios/teleios.h"
#include "teleios/profiler/types.inl"

// Linear allocator structures
typedef struct {
    TLMemoryTag tag;
    void* pointer;
} TLMemory;

typedef struct {
    char* payload;          // The memory block
    u32 size;               // The actual memory size
    u32 index;              // Available memory start position
} TLMemoryPage;

// Dynamic allocator structures
typedef struct TLDynamicBlock {
    void* pointer;
    struct TLDynamicBlock* next;
    u32 size;
    TLMemoryTag tag;
#ifdef TELEIOS_BUILD_DEBUG
    TLStackTrace stack_trace;
#endif
} TLDynamicBlock;

struct TLAllocator {
    union {
        struct {
            TLMemoryPage* page;
            u16 page_count;
        } linear;
        struct {
            TLDynamicBlock* head;
            u32 allocation_count;
        } dynamic;
    };
    TLAllocatorType type;
#if defined(TELEIOS_BUILD_DEBUG)
    TLStackTrace stack_trace;
#endif
};

inline const char* tl_memory_type_name(const TLMemoryTag tag) {
    switch (tag) {
        case TL_MEMORY_CONTAINER_POOL: return "TL_MEMORY_CONTAINER_POOL";
        case TL_MEMORY_BLOCK: return "TL_MEMORY_BLOCK";
        case TL_MEMORY_CONTAINER_ITERATOR: return "TL_MEMORY_CONTAINER_ITERATOR";
        case TL_MEMORY_GRAPHICS: return "TL_MEMORY_GRAPHICS";
        case TL_MEMORY_SERIALIZER: return "TL_MEMORY_SERIALIZER";
        case TL_MEMORY_CONTAINER_QUEUE: return "TL_MEMORY_CONTAINER_QUEUE";
        case TL_MEMORY_CONTAINER_STACK: return "TL_MEMORY_CONTAINER_STACK";
        case TL_MEMORY_CONTAINER_LIST: return "TL_MEMORY_CONTAINER_LIST";
        case TL_MEMORY_CONTAINER_NODE: return "TL_MEMORY_CONTAINER_NODE";
        case TL_MEMORY_CONTAINER_MAP: return "TL_MEMORY_CONTAINER_MAP";
        case TL_MEMORY_CONTAINER_MAP_ENTRY: return "TL_MEMORY_CONTAINER_MAP_ENTRY";
        case TL_MEMORY_STRING: return "TL_MEMORY_STRING";
        case TL_MEMORY_ULID: return "TL_MEMORY_ULID";
        case TL_MEMORY_PROFILER: return "TL_MEMORY_PROFILER";
        case TL_MEMORY_SCENE: return "TL_MEMORY_SCENE";
        case TL_MEMORY_ECS_COMPONENT: return "TL_MEMORY_ECS_COMPONENT";
        case TL_MEMORY_THREAD: return "TL_MEMORY_THREAD";
        case TL_MEMORY_MAXIMUM: return "TL_MEMORY_MAXIMUM";
    }

    return "??";
}

inline const char* tl_memory_allocator_name(const TLAllocatorType type) {
    switch (type) {
        case TL_ALLOCATOR_LINEAR: return "TL_ALLOCATOR_LINEAR";
        case TL_ALLOCATOR_DYNAMIC: return "TL_ALLOCATOR_DYNAMIC";
    }
    return "??";
}

#endif