#include "libk/libk.h"

typedef struct {
    u64 index;
    char *buffer;
} KMemoryPage;

struct KAllocator {
    KAllocatorType type;
    u64 page_size;
    u64 allocated;
    u64 tagged_size[U16_MAX];
    u64 tagged_count[U16_MAX];
    KMemoryPage page[U8_MAX];
};

KAllocator* k_memory_allocator_create(const KAllocatorType type, const u64 size) {
    K_FRAME_PUSH_WITH("%d, %llu", type, size)
    KAllocator *allocator = k_memory_alloc(sizeof(KAllocator));
    k_memory_set(allocator, 0, sizeof(KAllocator));

    allocator->type = type;
    allocator->page_size = size;

    K_FRAME_POP_WITH(allocator)
}

static void* k_memory_linear_alloc(KAllocator *allocator, u64 size) {
    K_FRAME_PUSH_WITH("0x%p, %llu, %d", allocator, size)
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (size == 0) {
        KFATAL("KAllocator 0x%p allocation size must be greater then 0", allocator)
        K_FRAME_POP_WITH(NULL)
    }

    if (size > allocator->page_size) {
        KFATAL("KAllocator with page size of %d bytes. It cannot allocate %d bytes", allocator, allocator->page_size, size)
        K_FRAME_POP_WITH(NULL)
    }
    // -------------------------------------------------
    // Find a suitable KMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {
        if (allocator->page[i].buffer == NULL) {
            allocator->page[i].buffer = k_memory_alloc(allocator->page_size);
            k_memory_set(allocator->page[i].buffer, 0, allocator->page_size);

            found = i;
            break;
        }

        // check if the page support the desired size
        if (allocator->page[i].index + size <= allocator->page_size) {
            found = i;
            break;
        }
    }

    if (found == U8_MAX) K_FRAME_POP_WITH(NULL)
    // -------------------------------------------------
    // Adjust the TLMemoryPage internal state
    // -------------------------------------------------
    void* address = allocator->page[found].buffer + allocator->page[found].index;
    allocator->page[found].index += size;
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    K_FRAME_POP_WITH(address)
}

void* k_memory_allocator_alloc(KAllocator *allocator, const u64 size, const u16 tag) {
    K_FRAME_PUSH_WITH("0x%p, %llu, %d", allocator, size)
    // -------------------------------------------------
    // Delegate the allocation
    // -------------------------------------------------
    void *pointer = NULL;
    switch (allocator->type) {
        default: KFATAL("Implementation missing")
        case K_MEMORY_ALLOCATOR_LINEAR: pointer = k_memory_linear_alloc(allocator, size); break;
    }
    // -------------------------------------------------
    // Adjust the KAllocator internal state
    // -------------------------------------------------
    allocator->allocated += size;
    allocator->tagged_count[tag] += 1;
    allocator->tagged_size[tag] += size;
    K_FRAME_POP_WITH(pointer)

}
void* k_memory_allocator_free(KAllocator *allocator, void *pointer) {
    switch (allocator->type) {
        case K_MEMORY_ALLOCATOR_DYNAMIC: KFATAL("Implementation missing")
        case K_MEMORY_ALLOCATOR_LINEAR: KFATAL("Unsupported Operation")
    }

    KFATAL("Unknown allocator type")
}

void k_memory_allocator_reset(KAllocator *allocator) {
    K_FRAME_PUSH_WITH("0x%p", allocator)

    for (u32 i = 0 ; i < U8_MAX ; ++i) {
        if (allocator->page[i].buffer == NULL) break;

        allocator->page[i].index = 0;
        k_memory_set(allocator->page[i].buffer, 0, allocator->page_size);
    }

    K_FRAME_POP
}

void k_memory_allocator_destroy(KAllocator* allocator) {
    K_FRAME_PUSH_WITH("0x%p", allocator)
    for (u32 i = 0 ; i < U8_MAX ; ++i) {
        if (allocator->page[i].buffer == NULL) break;
        k_memory_free(allocator->page + i);
    }

    k_memory_free(allocator);
    K_FRAME_POP
}