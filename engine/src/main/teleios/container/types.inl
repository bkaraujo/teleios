#ifndef __TELEIOS_CONTAINER_TYPES__
#define __TELEIOS_CONTAINER_TYPES__

#include "teleios/defines.h"

struct TLArray {
    void* items;            // Array of void pointers (payloads)
    u32 count;              // Current number of items
    u64 stride;             // Concrete item length

    TLMutex* mutex;         // Thread-safety
    TLCondition* not_empty; // Signals when items are available
    TLCondition* not_full;  // Signals when space is available

    TLAllocator* allocator; // Memory allocator for cleanup
};

// ---------------------------------
// Queue Implementation (Circular Buffer)
// ---------------------------------

struct TLQueue {
    void** items;           // Array of void pointers (payloads)
    u16 capacity;           // Maximum number of items
    u16 head;               // Next slot for insertion
    u16 tail;               // Next slot for removal
    u16 count;              // Current number of items
    u32 mod_count;          // Modification counter for fail-fast iteration

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
    u32 mod_count;          // Modification counter for fail-fast iteration
    TLMutex* mutex;         // Thread-safety for acquire/release operations
    TLAllocator* allocator; // Memory allocator for cleanup
};

// ---------------------------------
// Double Linked List Implementation
// ---------------------------------

struct TLListNode {
    void* data;             // Payload stored in this node
    TLListNode* prev; // Previous node in list
    TLListNode* next; // Next node in list
};

struct TLList {
    TLListNode* head;       // First node in list
    TLListNode* tail;       // Last node in list
    u32 size;               // Current number of nodes
    u32 mod_count;          // Modification counter for fail-fast iteration
    TLMutex* mutex;         // Thread-safety
    TLAllocator* allocator; // Memory allocator for cleanup
};

// ---------------------------------
// HashMap Implementation (TLString -> TLList*)
// ---------------------------------

struct TLMapEntry {
    TLString* key;          // String key (owned by this entry)
    TLList* value;          // List of void* (owned by this entry)
    TLMapEntry* next; // Next entry in bucket (chaining for collision resolution)
};

struct TLMap {
    TLMapEntry** buckets;   // Array of bucket pointers (separate chaining)
    u32 capacity;           // Number of buckets
    u32 size;               // Number of key-value pairs
    u32 mod_count;          // Modification counter for fail-fast iteration
    f32 load_factor;        // Maximum load factor before resize
    TLMutex* mutex;         // Thread-safety
    TLAllocator* allocator; // Memory allocator for cleanup
};

// ---------------------------------
// Iterator Implementation
// ---------------------------------

struct TLIterator {
    void* source;           // Pointer to container being iterated (TLList*, TLMap*, TLQueue*, or TLObjectPool*)
    u32 expected_mod_count; // Expected modification count (for fail-fast)
    u32 size;               // Total number of items in container

    // Function pointers for polymorphic behavior
    void    (*has_modified  )(const TLIterator* iterator);  // Check for concurrent modifications
    b8      (*has_next      )(const TLIterator* iterator);  // Check if more items available
    void*   (*next          )(TLIterator* iterator      );  // Get next item and advance
    void    (*rewind        )(TLIterator* iterator      );  // Reset to beginning
    void    (*resync        )(TLIterator* iterator      );  // Resynchronize with container state

    // State specific to container type (allocated by container, freed by iterator)
    void* state;

    // Allocator used to free the iterator and state
    TLAllocator* allocator;
};

#endif