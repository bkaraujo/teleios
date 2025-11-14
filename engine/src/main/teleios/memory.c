#include "teleios/teleios.h"
#include "teleios/memory_types.inl"
#include "teleios/memory_linear.inl"
#include "teleios/memory_dynamic.inl"

static u16 m_allocators_capacity = 0;
static u16 m_allocators_count = 0;
static TLAllocator** m_allocators = NULL;

/* extern */ TLAllocator* g_allocator;

void* tl_malloc(const u32 size, const char* error_message) {
    TL_PROFILER_PUSH_WITH("%d, %s", size, error_message)
    void * memory = malloc(size);
    if (memory == NULL) TLFATAL(error_message)
    memset(memory, 0, size);
    TL_PROFILER_POP_WITH(memory)
}

b8 tl_memory_initialize(void){
    TL_PROFILER_PUSH

    m_allocators_capacity = 16;  // Initial capacity
    m_allocators_count = 0;
    m_allocators = tl_malloc(sizeof(TLAllocator*) * m_allocators_capacity, "Failed to allocate TLAllocator* array");

    g_allocator = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);

    TL_PROFILER_POP_WITH(true)
}

TLAllocator* tl_memory_allocator_create(const u32 size, const TLAllocatorType type){
    TL_PROFILER_PUSH_WITH("%u, %s", size, tl_memory_allocator_name(type))

    // Check if we need to grow the allocators array
    if (m_allocators_count >= m_allocators_capacity) {
        const u16 new_capacity = m_allocators_capacity * 2;
        TLAllocator** new_array = tl_malloc(sizeof(TLAllocator*) * new_capacity, "Failed to grow TLAllocator* array");

        // Copy existing allocators to new array
        if (m_allocators != NULL) {
            memcpy(new_array, m_allocators, sizeof(TLAllocator*) * m_allocators_count);
            free(m_allocators);
        }

        m_allocators = new_array;
        m_allocators_capacity = new_capacity;
        TLDEBUG("Allocators array grown to capacity %u", new_capacity);
    }

    // Allocate allocator individually on heap (prevents pointer invalidation)
    TLAllocator* allocator = tl_malloc(sizeof(TLAllocator), "Failed to allocate TLAllocator");
    allocator->type = type;

#if defined(TELEIOS_BUILD_DEBUG)
    // Capture stack trace at creation time for debugging dangling allocators
    tl_profiler_stacktrace_snapshot(&allocator->stack_trace);
#endif

    if (type == TL_ALLOCATOR_LINEAR) {
        if (size == 0) TLFATAL("LINEAR allocator requires size > 0")

        allocator->linear.page_count = 1;
        allocator->linear.page = tl_malloc(sizeof(TLMemoryPage), "Failed to allocate TLMemoryPage");
        allocator->linear.page->size = size;
        allocator->linear.page->payload = tl_malloc(size, "Failed to allocate TLMemoryPage->payload");

        TLTRACE("LINEAR allocator created:0x%p (page_size=%u)", allocator, size);
    } else {
        if (size > 0) TLWARN("DYNAMIC allocator does not requires a size")
        TLTRACE("DYNAMIC allocator created:0x%p", allocator);
    }

    // Add allocator to tracking array
    m_allocators[m_allocators_count++] = allocator;

    TL_PROFILER_POP_WITH(allocator)
}

void tl_memory_allocator_destroy(TLAllocator* allocator){
    TL_PROFILER_PUSH_WITH("0x%p", allocator)

    if (allocator == NULL) TLFATAL("allocator is NULL")

    // Remove from tracking array
    for (u16 i = 0; i < m_allocators_count; i++) {
        if (m_allocators[i] == allocator) {
            if (i < m_allocators_count - 1) {
                const u32 bytes_to_move = sizeof(TLAllocator*) * (m_allocators_count - i - 1);
                memcpy(&m_allocators[i], &m_allocators[i + 1], bytes_to_move);
            }

            m_allocators[--m_allocators_count] = NULL;
            break;
        }
    }

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

    // Free the allocator structure itself (allocated on heap)
    TLTRACE("LINEAR allocator destroyed:0x%p", allocator);
    free(allocator);

    TL_PROFILER_POP
}

void* tl_memory_alloc(TLAllocator* allocator, const TLMemoryTag tag, const u32 size){
    TL_PROFILER_PUSH_WITH("0x%p, %d, %u", allocator, tag, size)

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
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", allocator, pointer)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (pointer == NULL) {
        TLWARN("Attempted to free NULL pointer");
        TL_PROFILER_POP
    }

    switch (allocator->type) {
        case TL_ALLOCATOR_LINEAR:
            break;
        case TL_ALLOCATOR_DYNAMIC:
            tl_memory_dynamic_free(allocator, pointer);
            break;
        default:
            TLFATAL("Unsupported Allocator type %d", allocator->type);
    }

    TL_PROFILER_POP
}

void tl_memory_set(void *target, const i32 value, const u32 size){
    TL_PROFILER_PUSH_WITH("0x%p, %d, %u", target, value, size)

    if (size == 0) TLFATAL("size is 0")
    if (target == NULL) TLFATAL("target is NULL")
    
    memset(target, value, size);

    TL_PROFILER_POP
}

void tl_memory_copy(void *target, const void *source, const u32 size){
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, %u", target, source, size)

    if (size == 0) TLFATAL("size is 0")
    if (source == NULL) TLFATAL("source is NULL")
    if (target == NULL) TLFATAL("target is NULL")

    memcpy(target, source, size);
    TL_PROFILER_POP
}

b8 tl_memory_terminate(void) {
    TL_PROFILER_PUSH

    tl_memory_allocator_destroy(g_allocator);

    // Release all dangling allocators
    while (m_allocators_count > 0) {
        TLAllocator* allocator = m_allocators[m_allocators_count - 1];
        TLWARN("Releasing dangling allocator 0x%p", allocator)
#if defined(TELEIOS_BUILD_DEBUG)
        tl_profiler_stacktrace_print(&allocator->stack_trace);
#endif
        tl_memory_allocator_destroy(allocator);
    }

    if (m_allocators != NULL) {
        free(m_allocators);
        m_allocators = NULL;
    }

    // Reset state
    m_allocators_capacity = 0;
    m_allocators_count = 0;

    TLDEBUG("Memory system terminated successfully");

    TL_PROFILER_POP_WITH(true)
}
