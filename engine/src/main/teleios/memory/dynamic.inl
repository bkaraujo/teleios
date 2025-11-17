#ifndef __TELEIOS_MEMORY_DYNAMIC__
#define __TELEIOS_MEMORY_DYNAMIC__

#include "teleios/teleios.h"
#include "teleios/memory/types.inl"

// Forward declaration from memory.c
extern void* tl_malloc(u32 size, const char* error_message);

// ---------------------------------
// DYNAMIC allocator - allocate from heap and track
// ---------------------------------
static void* tl_memory_dynamic_alloc(TLAllocator* allocator, const TLMemoryTag tag, const u32 size) {
    TL_PROFILER_PUSH_WITH("0x%p, %s, %u", allocator, tl_memory_type_name(tag), size)

    TLDynamicBlock* block = (TLDynamicBlock*)tl_malloc(sizeof(TLDynamicBlock), "Failed to allocate TLDynamicBlock");
    block->tag = tag;
    block->size = size;
    block->next = allocator->dynamic.head;
    block->pointer = tl_malloc(size, "Failed to allocate");
#ifdef TELEIOS_BUILD_DEBUG
    tl_profiler_stacktrace_snapshot(&block->stack_trace);
#endif
    // Insert at head of linked list
    allocator->dynamic.head = block;
    allocator->dynamic.allocation_count++;

    TLVERBOSE("DYNAMIC alloc: %u bytes (ptr=0x%p, total=%u, tag=%s)",
        size, block->pointer, allocator->dynamic.allocation_count,
        tl_memory_type_name(tag));

    TL_PROFILER_POP_WITH(block->pointer)
}

// ---------------------------------
// DYNAMIC allocator - free individual allocation
// ---------------------------------
static void tl_memory_dynamic_free(TLAllocator* allocator, void* pointer) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", allocator, pointer)

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

            TLVERBOSE("DYNAMIC free: %u bytes (ptr=0x%p, remaining=%u, tag=%s)",
                current->size, pointer, allocator->dynamic.allocation_count, tl_memory_type_name(current->tag));

            free(current->pointer);
            free(current);
            TL_PROFILER_POP
        }

        prev = current;
        current = current->next;
    }

    TLERROR("Pointer0x%p not found in DYNAMIC allocator 0x%p", pointer, allocator);
    TL_PROFILER_POP
}

// ---------------------------------
// DYNAMIC allocator - destroy and report leaks
// ---------------------------------
static void tl_memory_dynamic_destroy(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)
    if (allocator == NULL) TLFATAL("TLAllocator is NULL")

    // Check for memory leaks in DYNAMIC allocator
    if (allocator->dynamic.head != NULL) {
        u32 leak_count = 0;
        u32 leaked_bytes = 0;

        TLWARN("Memory leak detected in allocator 0x%p:", allocator);

        TLDynamicBlock* block = allocator->dynamic.head;
        while (block != NULL) {
            leak_count++;
            leaked_bytes += block->size;
            TLWARN("  Leak #%u: %u bytes (ptr=0x%p, tag=%s)", leak_count, block->size, block->pointer, tl_memory_type_name(block->tag));
#ifdef TELEIOS_BUILD_DEBUG
            tl_profiler_stacktrace_print(&block->stack_trace);
#endif
            TLDynamicBlock* next = block->next;
            free(block->pointer);
            free(block);
            block = next;
        }

        TLERROR("Total memory leaks: %u allocations, %u bytes", leak_count, leaked_bytes);
    }

    allocator->dynamic.head = NULL;
    allocator->dynamic.allocation_count = 0;
    TL_PROFILER_POP
}

#endif