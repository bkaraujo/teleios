#include "teleios/teleios.h"
#include "teleios/graphics_types.inl"
#include "teleios/container_types.inl"

#include "teleios/graphics_queue.inl"
#include "teleios/graphics_thread.inl"

// ---------------------------------
// Event Handlers
// ---------------------------------

static TLEventStatus tl_graphics_handle_window_closed(const TLEvent* event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    TLDEBUG("Signaling graphics thread to terminate")
    TLGraphicTask task = { 0 };
    tl_queue_push(m_queue, &task);

    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

// ---------------------------------
// Lifecycle API
// ---------------------------------

#define TL_GRAPHICS_QUEUE_CAPACITY 512

TLAllocator* m_allocator = NULL;  // Accessible from graphics_queue.inl
static TLThread* m_worker_thread = NULL;

b8 tl_graphics_initialize(void) {
    TL_PROFILER_PUSH

    if (!tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_graphics_handle_window_closed)) {
        TLERROR("Failed to subscribe to window closed event")
        TL_PROFILER_POP_WITH(false)
    }

    m_allocator = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
    m_queue = tl_queue_create(m_allocator, TL_GRAPHICS_QUEUE_CAPACITY);

    m_worker_thread = tl_thread_create(tl_graphics_worker, NULL);
    if (!m_worker_thread) {
        TLFATAL("Failed to create graphics worker thread")
    }

    TL_PROFILER_POP_WITH(true)
}

b8 tl_graphics_terminate(void) {
    TL_PROFILER_PUSH

    if (!m_queue || !m_worker_thread) {
        TL_PROFILER_POP_WITH(true)
    }

    // Wait for worker thread to finish
    tl_thread_join(m_worker_thread, NULL);
    m_worker_thread = NULL;

    // Cleanup
    if (m_queue) {
        TLTRACE("Destroying queue 0x%p", m_queue)
        tl_queue_destroy(m_queue);
        m_queue = NULL;
    }

    TLTRACE("Destroying allocator 0x%p", m_allocator)
    tl_memory_allocator_destroy(m_allocator);
    m_allocator = NULL;

    TL_PROFILER_POP_WITH(true)
}
