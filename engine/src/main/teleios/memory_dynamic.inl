#ifndef __TELEIOS_MEMORY_DYNAMIC__
#define __TELEIOS_MEMORY_DYNAMIC__

#include "teleios/memory_types.inl"

// ---------------------------------
// DYNAMIC allocator - allocate from heap and track
// ---------------------------------
static void* tl_memory_dynamic_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size) {
    TL_PROFILER_PUSH_WITH("%p, %d, %u", allocator, tag, size)

    TLDynamicBlock* block = (TLDynamicBlock*)tl_malloc(sizeof(TLDynamicBlock), "Failed to allocate TLDynamicBlock");
    block->tag = tag;
    block->size = size;
    block->next = allocator->dynamic.head;
    block->pointer = tl_malloc(size, "Failed to allocate");

    // Insert at head of linked list
    allocator->dynamic.head = block;
    allocator->dynamic.allocation_count++;

    TLVERBOSE("DYNAMIC alloc: %u bytes (tag=%d, ptr=%p, total=%u)", size, tag, block->pointer, allocator->dynamic.allocation_count);

    TL_PROFILER_POP_WITH(block->pointer)
}

// ---------------------------------
// DYNAMIC allocator - free individual allocation
// ---------------------------------
static void tl_memory_dynamic_free(TLAllocator* allocator, void* pointer) {
    TL_PROFILER_PUSH_WITH("%p, %p", allocator, pointer)

    // Search for block in linked list
    TLDynamicBlock* prev = NULL;
    TLDynamicBlock* current = allocator->dynamic.head;

    while (current != NULL) {
        if (current->pointer == pointer) {
            // Found the block, remove from list
            if (prev == NULL) {
                allocator->dynamic.head = current->next;
            } else {
                prev->next = current->next;
            }

            allocator->dynamic.allocation_count--;

            TLVERBOSE("DYNAMIC free: %u bytes (tag=%d, ptr=%p, remaining=%u)", current->size, current->tag, pointer, allocator->dynamic.allocation_count);

            free(current->pointer);
            free(current);
            TL_PROFILER_POP
        }

        prev = current;
        current = current->next;
    }

    TLERROR("Pointer %p not found in DYNAMIC allocator %p", pointer, allocator);
    TL_PROFILER_POP
}

// ---------------------------------
// DYNAMIC allocator - destroy and report leaks
// ---------------------------------
static void tl_memory_dynamic_destroy(TLAllocator* allocator) {
    TL_PROFILER_PUSH
    if (allocator == NULL) TLFATAL("TLAllocator is NULL")

    // Check for memory leaks in DYNAMIC allocator
    if (allocator->dynamic.head != NULL) {
        u32 leak_count = 0;
        u32 leaked_bytes = 0;

        TLWARN("Memory leaks detected in DYNAMIC allocator %p:", allocator);

        TLDynamicBlock* block = allocator->dynamic.head;
        while (block != NULL) {
            leak_count++;
            leaked_bytes += block->size;
            TLWARN("  Leak #%u: %u bytes (tag=%d, ptr=%p)",
                leak_count, block->size, block->tag, block->pointer);

            TLDynamicBlock* next = block->next;
            free(block->pointer);
            free(block);
            block = next;
        }

        TLERROR("Total memory leaks: %u allocations, %u bytes", leak_count, leaked_bytes);
    } else {
        TLDEBUG("DYNAMIC allocator destroyed: %p (no leaks)", allocator);
    }

    allocator->dynamic.head = NULL;
    allocator->dynamic.allocation_count = 0;
    TL_PROFILER_POP
}

#endif