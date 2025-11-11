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

    // Watch for window closing event so we can join the graphics thread
    if (!tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_graphics_handle_window_closed)) {
        TLERROR("Failed to subscribe to window closed event")
        TL_PROFILER_POP_WITH(false)
    }

    m_allocator = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
    m_queue = tl_queue_create(m_allocator, TL_GRAPHICS_QUEUE_CAPACITY);

    // Create object pool for graphics tasks
    m_task_pool = tl_pool_create(m_allocator, sizeof(TLGraphicTask), TL_GRAPHICS_QUEUE_CAPACITY);
    if (!m_task_pool) {
        TLFATAL("Failed to create graphics task pool")
    }

    // Pre-allocate mutex and condition variable for each task in the pool
    for (u16 i = 0; i < TL_GRAPHICS_QUEUE_CAPACITY; ++i) {
        TLGraphicTask* task = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
        if (!task) {
            TLFATAL("Failed to acquire task from pool during initialization")
        }

        task->completion_mutex = tl_mutex_create(m_allocator);
        task->completion_condition = tl_condition_create(m_allocator);

        if (!task->completion_mutex || !task->completion_condition) {
            TLFATAL("Failed to create synchronization primitives for graphics task")
        }

        // Release back to pool - task is now ready for use
        tl_pool_release(m_task_pool, task);
    }

    TLINFO("Graphics task pool initialized: %u tasks pre-allocated", TL_GRAPHICS_QUEUE_CAPACITY)

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

    // Destroy pre-allocated synchronization primitives in task pool
    if (m_task_pool) {
        TLTRACE("Destroying graphics task pool synchronization primitives")

        // Reset pool to ensure all tasks are available
        tl_pool_reset(m_task_pool);

        // Destroy mutex and condition variable for each task
        for (u16 i = 0; i < TL_GRAPHICS_QUEUE_CAPACITY; ++i) {
            TLGraphicTask* task = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
            if (task == NULL) continue;

            if (task->completion_mutex) {
                tl_mutex_destroy(task->completion_mutex);
                task->completion_mutex = NULL;
            }

            if (task->completion_condition) {
                tl_condition_destroy(task->completion_condition);
                task->completion_condition = NULL;
            }
        }

        TLTRACE("Destroying task pool 0x%p", m_task_pool)
        tl_pool_destroy(m_task_pool);
        m_task_pool = NULL;
    }

    // Cleanup queue
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
