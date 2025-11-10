#include "teleios/teleios.h"
#include "teleios/memory_types.inl"
#include "teleios/memory_linear.inl"
#include "teleios/memory_dynamic.inl"

static u16 m_allocators_count = 1;
static TLAllocator* m_allocators;

// Helper function for memory allocation (used by .inl files)
// Not static to allow access from included .inl files
void* tl_malloc(const u32 size, const char* message) {
    TL_PROFILER_PUSH
    void * memory = malloc(size);
    if (memory == NULL) TLFATAL(message)
    memset(memory, 0, size);
    TL_PROFILER_POP_WITH(memory)
}

b8 tl_memory_initialize(void){
    TL_PROFILER_PUSH

    m_allocators = tl_malloc(sizeof(TLAllocator) * m_allocators_count, "Failed to allocate TLAllocator* array");

    TL_PROFILER_POP_WITH(true)
}

TLAllocator* tl_memory_allocator_create(u32 size, TLAllocatorType type){
    TL_PROFILER_PUSH

    TLAllocator* allocator = NULL;

    // Try to find an empty slot
    for (u16 i = 0 ; i < m_allocators_count ; ++i) {
        allocator = m_allocators + i;
        if (allocator->type == TL_ALLOCATOR_LINEAR && allocator->linear.page == NULL) {
            goto initialize_allocator;
        }
        if (allocator->type == TL_ALLOCATOR_DYNAMIC && allocator->dynamic.head == NULL) {
            goto initialize_allocator;
        }
    }

    // No empty slot found, expand array
    u16 new_size = (u16)(m_allocators_count * 1.75f) + 1;
    TLAllocator* new_m_allocators = tl_malloc(sizeof(TLAllocator) * new_size, "Failed to allocate TLAllocator* array");
    memcpy(new_m_allocators, m_allocators, sizeof(TLAllocator) * m_allocators_count);

    free(m_allocators);
    allocator = new_m_allocators + m_allocators_count;
    m_allocators = new_m_allocators;
    m_allocators_count = new_size;

initialize_allocator:
    allocator->type = type;

    if (type == TL_ALLOCATOR_LINEAR) {
        if (size == 0) TLFATAL("LINEAR allocator requires size > 0")

        allocator->linear.page_count = 1;
        allocator->linear.page = tl_malloc(sizeof(TLMemoryPage), "Failed to allocate TLMemoryPage");
        allocator->linear.page->size = size;
        allocator->linear.page->index = 0;
        allocator->linear.page->payload = tl_malloc(size, "Failed to allocate TLMemoryPage->payload");

        TLDEBUG("LINEAR allocator created: %p (page_size=%u)", allocator, size);
    } else {
        if (size > 0) TLERROR("DYNAMIC allocator does not requires a size")
        allocator->dynamic.head = NULL;
        allocator->dynamic.allocation_count = 0;

        TLDEBUG("DYNAMIC allocator created: %p", allocator);
    }

    TL_PROFILER_POP_WITH(allocator)
}

void tl_memory_allocator_destroy(TLAllocator* allocator){
    TL_PROFILER_PUSH

    if (allocator == NULL) TLFATAL("allocator is NULL")

    switch (allocator->type) {
        case TL_ALLOCATOR_LINEAR:
            tl_memory_linear_destroy(allocator);
            break;
        case TL_ALLOCATOR_DYNAMIC:
            tl_memory_dynamic_destroy(allocator);
            break;
        default:
            TLFATAL("Unsupported Allocator type %d", allocator->type);
    }

    TL_PROFILER_POP
}

void* tl_memory_alloc(TLAllocator* allocator, TLMemoryTag tag, u32 size){
    TL_PROFILER_PUSH_WITH("%p, %d, %u", allocator, tag, size)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (size == 0) TLFATAL("size is 0")

    void* memory = NULL;

    switch (allocator->type) {
        case TL_ALLOCATOR_LINEAR:
            memory = tl_memory_linear_alloc(allocator, tag, size);
            break;
        case TL_ALLOCATOR_DYNAMIC:
            memory = tl_memory_dynamic_alloc(allocator, tag, size);
            break;
        default:
            TLFATAL("Unsupported Allocator type %d", allocator->type);
    }

    TL_PROFILER_POP_WITH(memory)
}

void tl_memory_free(TLAllocator* allocator, void* pointer){
    TL_PROFILER_PUSH_WITH("%p, %p", allocator, pointer)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (pointer == NULL) {
        TLWARN("Attempted to free NULL pointer");
        TL_PROFILER_POP
    }

    switch (allocator->type) {
        case TL_ALLOCATOR_LINEAR:
            TLERROR("Cannot free individual allocations from LINEAR allocator %p", allocator);
            break;
        case TL_ALLOCATOR_DYNAMIC:
            tl_memory_dynamic_free(allocator, pointer);
            break;
        default:
            TLFATAL("Unsupported Allocator type %d", allocator->type);
    }

    TL_PROFILER_POP
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
