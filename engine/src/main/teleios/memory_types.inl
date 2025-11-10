#ifndef __TELEIOS_MEMORY_TYPES__
#define __TELEIOS_MEMORY_TYPES__

#include "teleios/teleios.h"

// Linear allocator structures
typedef struct {
    TLMemoryTag tag;
    void* pointer;
} TLMemory;

typedef struct {
    u32 size;               // The actual memory size
    u32 index;              // Available memory start position
    char* payload;          // The memory block
} TLMemoryPage;

// Dynamic allocator structures
typedef struct TLDynamicBlock {
    TLMemoryTag tag;
    void* pointer;
    u32 size;
    struct TLDynamicBlock* next;
} TLDynamicBlock;

struct TLAllocator {
    TLAllocatorType type;
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
};

#endif