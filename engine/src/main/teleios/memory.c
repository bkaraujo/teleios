#include "teleios/teleios.h"

// #################################################################
// TLAllocator is a linear alocator composed of infinite TLMemoryPage
// whose responsibility is to hold the TLMemory requested of it. The 
// TLMemory has the actual memory pointer and the TLMemoryTag that
// clarify what that pointer is used for.
// #################################################################
typedef struct {
    TLMemoryTag tag;
    void* pointer;
} TLMemory;

typedef struct {
    u32 size;               // The actual memory size
    u32 index;              // Avaliable memory start position
    char* payload;          // The memory block
} TLMemoryPage;

struct TLAllocator {
    TLMemoryPage* page;
    u16 page_count;
};

static u16 allocators_count = 1;
static TLAllocator* allocators;

static TL_INLINE void* tl_malloc(const u32 size, const char* message) {
    void * memory = malloc(size);
    if (memory == NULL) TLFATAL(message)
    memset(memory, 0, size);
    return memory;
}

b8 tl_memory_initialize(void){
    TL_PROFILER_PUSH

    allocators = tl_malloc(sizeof(TLAllocator) * allocators_count, "Failed to allocate TLAllocator* array");

    TL_PROFILER_POP_WITH(true)
}

TLAllocator* tl_memory_allocator_create(u32 size){
    TL_PROFILER_PUSH
    
    for (u16 i = 0 ; i < allocators_count ; ++i) {
        TLAllocator* allocator = (allocators + i);
        if (allocator->page_count == 0) {
            allocator->page_count = 1;
            allocator->page = tl_malloc(sizeof(TLMemoryPage), "Failed to allocate TLMemoryPage");
            allocator->page->size = size;
            allocator->page->payload = tl_malloc(size, "Failed to allocate TLMemoryPage->payload");
            TL_PROFILER_POP_WITH(allocator)
        }
    }

    u16 new_size = (u16)(allocators_count * 1.75f) + 1;
    TLAllocator* new_allocators = tl_malloc(sizeof(TLAllocator*) * allocators_count, "Failed to allocate TLAllocator* array");
    memcpy(new_allocators, allocators, sizeof(TLAllocator*) * allocators_count);
    
    TLAllocator* allocator = (allocators + allocators_count);
    allocators = new_allocators;
    allocators_count = new_size;

    TL_PROFILER_POP_WITH(allocator)
}

void tl_memory_allocator_destroy(TLAllocator* allocator){
    TL_PROFILER_PUSH
    
    if (allocator == NULL) TLFATAL("TLAllocator is NULL")

    for (u16 i = 0 ; i < allocator->page_count ; ++i) {
        TLMemoryPage* page = allocator->page + i;
        if (page->payload != NULL) {
            free(page->payload);
            page->payload = NULL;
        }
        free(page);
        page = NULL;
    }

    free(allocator);
    allocator = NULL;

    TL_PROFILER_POP
}

TL_INLINE static void* tl_memory_allocate(TLAllocator* allocator, TLMemoryTag tag, u32 size) {
    for (u16 i = 0 ; i < allocator->page_count ; ++i) {
        TLMemoryPage* page = allocator->page + i;
        u32 available = page->size - page->index;
        if (available < size) continue;

        TLMemory* block = (TLMemory*) (page->payload + page->index);
        block->tag = tag;
        
        page->index += size;
        page->index += sizeof(TLMemoryTag);
        
       return (block + sizeof(TLMemoryTag));
    }

    return NULL;
}

TL_INLINE void static tl_memory_resize(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("%p", allocator)

    u16 new_page_count = (u16)(allocator->page_count * 1.75f) + 1;
    TLMemoryPage* new_pages = tl_malloc(sizeof(TLMemoryPage) * new_page_count, "Failed to resize TLAllocator");
    memcpy(new_pages, allocator->page, sizeof(TLMemoryPage) * allocator->page_count);

    free(allocator->page);
    allocator->page = NULL;
    allocator->page = new_pages;

    TL_PROFILER_POP
}

void* tl_memory_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size){
    TL_PROFILER_PUSH_WITH("%p, %d, %llu", allocator, tag, size)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (allocator->page->size < size) TLFATAL("TLAllocator can't allocate %d bytes", size);
    
    void* memory = tl_memory_allocate(allocator, tag, size);
    if (memory != NULL) TL_PROFILER_POP_WITH(memory)

    tl_memory_resize(allocator);

    memory = tl_memory_allocate(allocator, tag, size);
    TL_PROFILER_POP_WITH(memory)
}

void tl_memory_set(void *target, i32 value, u32 size){
    TL_PROFILER_PUSH_WITH("0x%p, %d, %llu", target, value, size)

    if (size == 0) TLFATAL("size is 0")
    if (target == NULL) TLFATAL("target is NULL")
    
    memset(target, value, size);

    TL_PROFILER_POP
}

void tl_memory_copy(void *target, const void *source, u32 size){
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, %llu", target, source, size)

    if (size == 0) TLFATAL("size is 0")
    if (source == NULL) TLFATAL("source is NULL")
    if (target == NULL) TLFATAL("target is NULL")

    memcpy(target, source, size);
    TL_PROFILER_POP
}

b8 tl_memory_terminate(void) {
    TL_PROFILER_PUSH

    TL_PROFILER_POP_WITH(true)
}