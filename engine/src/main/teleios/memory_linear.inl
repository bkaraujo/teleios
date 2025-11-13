#ifndef __TELEIOS_MEMORY_LINEAR__
#define __TELEIOS_MEMORY_LINEAR__

#include "teleios/memory_types.inl"

// Forward declaration for tl_malloc (defined in memory.c)
extern void* tl_malloc(u32 size, const char* error_message);

// ---------------------------------
// LINEAR allocator - allocate from pages
// ---------------------------------
static void* tl_memory_linear_allocate(TLAllocator* allocator, const TLMemoryTag tag, const u32 size) {
    for (u16 i = 0 ; i < allocator->linear.page_count ; ++i) {
        TLMemoryPage* page = allocator->linear.page + i;
        const u32 available = page->size - page->index;
        if (available < size + sizeof(TLMemoryTag)) continue;

        TLTRACE("LINEAR alloc: size %d, tag %s", size, tl_memory_type_name(tag));
        TLMemory* block = (TLMemory*) (page->payload + page->index);
        block->tag = tag;

        page->index += sizeof(TLMemoryTag);
        void* result = page->payload + page->index;
        page->index += size;

        TLTRACE("LINEAR alloc:0x%p available %d", allocator, page->size - page->index)
        return result;
    }

    return NULL;
}

// ---------------------------------
// LINEAR allocator - resize page array
// ---------------------------------
static void tl_memory_linear_resize(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("0x%p", allocator)

    const u16 new_page_count = (u16)(allocator->linear.page_count * 1.75f) + 1;
    TLMemoryPage* new_pages = tl_malloc(sizeof(TLMemoryPage) * new_page_count, "Failed to resize LINEAR allocator");
    memcpy(new_pages, allocator->linear.page, sizeof(TLMemoryPage) * allocator->linear.page_count);

    // Initialize the new page with the same size as the first page
    TLMemoryPage* new_page = new_pages + allocator->linear.page_count;
    new_page->size = allocator->linear.page->size;
    new_page->index = 0;
    new_page->payload = tl_malloc(new_page->size, "Failed to allocate new page payload");

    free(allocator->linear.page);
    allocator->linear.page = new_pages;
    allocator->linear.page_count = new_page_count;

    TLTRACE("LINEAR allocator resized:0x%p (pages=%u)", allocator, new_page_count);

    TL_PROFILER_POP
}

// ---------------------------------
// LINEAR allocator - main allocation function
// ---------------------------------
static void* tl_memory_linear_alloc(TLAllocator* allocator, const TLMemoryTag tag, const u32 size) {
    TL_PROFILER_PUSH_WITH("0x%p, %s, %u", allocator, tl_memory_type_name(tag), size)

    if (allocator->linear.page->size < size) {
        TLFATAL("LINEAR allocator page size (%u) is smaller than requested size (%u)",
            allocator->linear.page->size, size);
    }

    void* memory = tl_memory_linear_allocate(allocator, tag, size);
    if (memory == NULL) {
        tl_memory_linear_resize(allocator);
        memory = tl_memory_linear_allocate(allocator, tag, size);
    }

    TL_PROFILER_POP_WITH(memory)
}

// ---------------------------------
// LINEAR allocator - destroy (no individual free support)
// ---------------------------------
static void tl_memory_linear_destroy(TLAllocator* allocator) {
    TL_PROFILER_PUSH
    if (allocator == NULL) TLFATAL("TLAllocator is NULL")

    for (u16 i = 0 ; i < allocator->linear.page_count ; ++i) {
        TLMemoryPage* page = allocator->linear.page + i;
        if (page->payload != NULL) {
            free(page->payload);
            page->payload = NULL;
        }
    }

    free(allocator->linear.page);
    allocator->linear.page = NULL;
    allocator->linear.page_count = 0;

    TLTRACE("LINEAR allocator destroyed:0x%p", allocator);

    TL_PROFILER_POP
}

#endif