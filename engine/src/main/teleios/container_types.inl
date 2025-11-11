#ifndef __TELEIOS_CONTAINER_TYPES__
#define __TELEIOS_CONTAINER_TYPES__

#include "teleios/defines.h"

// ---------------------------------
// Queue Implementation (Circular Buffer)
// ---------------------------------

struct TLQueue {
    void** items;           // Array of void pointers (payloads)
    u16 capacity;           // Maximum number of items
    u16 head;               // Next slot for insertion
    u16 tail;               // Next slot for removal
    u16 count;              // Current number of items

    TLMutex* mutex;         // Thread-safety
    TLCondition* not_empty; // Signals when items are available
    TLCondition* not_full;  // Signals when space is available

    TLAllocator* allocator; // Memory allocator for cleanup
};

// ---------------------------------
// Object Pool Implementation
// ---------------------------------

struct TLObjectPool {
    u8* memory;             // Contiguous memory block for all objects
    b8* in_use;             // Bitmap: true if object is acquired
    u32 object_size;        // Size of each object in bytes
    u16 capacity;           // Total number of objects
    u16 next_free;          // Hint: next potentially free object
    TLMutex* mutex;         // Thread-safety for acquire/release operations
    TLAllocator* allocator; // Memory allocator for cleanup
};

#endif