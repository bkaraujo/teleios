#include "teleios/teleios.h"
#include "teleios/graphics_types.inl"
#include <GLFW/glfw3.h>
#include <cglm/version.h>

// ---------------------------------
// Constants
// ---------------------------------

#define TL_GRAPHICS_QUEUE_CAPACITY 256

// ---------------------------------
// Module State
// ---------------------------------

static TLGraphicsQueue* m_queue = NULL;
static TLThread* m_worker_thread = NULL;
static TLAllocator* m_allocator = NULL;

static void* tl_graphics_worker(void*);

// ---------------------------------
// Public API
// ---------------------------------

static b8 tl_graphics_queue_push(TLGraphicsQueue* queue, const TLGraphicTask* job) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", queue, job)

    tl_mutex_lock(queue->mutex);

    // Wait for space if queue is full
    while (queue->count >= queue->capacity && !queue->shutdown) {
        TLWARN("Graphics queue full, waiting for space...")
        tl_condition_wait(queue->not_full, queue->mutex);
    }

    if (queue->shutdown) {
        tl_mutex_unlock(queue->mutex);
        TLWARN("Cannot push to queue: shutdown in progress")
        TL_PROFILER_POP_WITH(false)
    }

    // Insert job at head
    queue->tasks[queue->head] = *job;
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count++;

    // Signal worker that work is available
    tl_condition_signal(queue->not_empty);
    tl_mutex_unlock(queue->mutex);

    TL_PROFILER_POP_WITH(true)
}

void tl_graphics_submit_sync(void (*func)(void)) {
    TL_PROFILER_PUSH_WITH("0x%p", func)

    if (!m_queue || !func) {
        TLERROR("Graphics queue not initialized or invalid function pointer")
        TL_PROFILER_POP
    }

    // Create synchronization primitives for this job
    TLMutex* completion_mutex = tl_mutex_create();
    TLCondition* completion_condition = tl_condition_create();

    const TLGraphicTask job = {
        .type = TL_GRAPHICS_JOB_NO_ARGS,
        .func_no_args = func,
        .args = NULL,
        .is_sync = true,
        .completed = false,
        .completion_mutex = completion_mutex,
        .completion_condition = completion_condition
    };

    // Push job to queue
    if (!tl_graphics_queue_push(m_queue, &job)) {
        TLERROR("Failed to push synchronous job to graphics queue")
        tl_mutex_destroy(completion_mutex);
        tl_condition_destroy(completion_condition);
        TL_PROFILER_POP
    }

    // Wait for completion
    tl_mutex_lock(completion_mutex);
    while (!job.completed) {
        TLVERBOSE("Waiting")
        tl_condition_wait(completion_condition, completion_mutex);
    }
    tl_mutex_unlock(completion_mutex);

    // Cleanup
    tl_mutex_destroy(completion_mutex);
    tl_condition_destroy(completion_condition);

    TL_PROFILER_POP
}

void tl_graphics_submit_sync_args(void (*func)(void*), void* args) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, args)

    if (!m_queue || !func) {
        TLERROR("Graphics queue not initialized or invalid function pointer")
        TL_PROFILER_POP
    }

    // Create synchronization primitives for this job
    TLMutex* completion_mutex = tl_mutex_create();
    TLCondition* completion_condition = tl_condition_create();

    TLGraphicTask job = {
        .type = TL_GRAPHICS_JOB_WITH_ARGS,
        .func_with_args = func,
        .args = args,
        .is_sync = true,
        .completed = false,
        .completion_mutex = completion_mutex,
        .completion_condition = completion_condition
    };

    // Push job to queue
    if (!tl_graphics_queue_push(m_queue, &job)) {
        TLERROR("Failed to push synchronous job with args to graphics queue")
        tl_mutex_destroy(completion_mutex);
        tl_condition_destroy(completion_condition);
        TL_PROFILER_POP
    }

    // Wait for completion
    tl_mutex_lock(completion_mutex);
    while (!job.completed) {
        tl_condition_wait(completion_condition, completion_mutex);
    }
    tl_mutex_unlock(completion_mutex);

    // Cleanup
    tl_mutex_destroy(completion_mutex);
    tl_condition_destroy(completion_condition);

    TL_PROFILER_POP
}

void tl_graphics_submit_async(void (*func)(void)) {
    TL_PROFILER_PUSH_WITH("0x%p", func)

    if (!m_queue || !func) {
        TLERROR("Graphics queue not initialized or invalid function pointer")
        TL_PROFILER_POP
    }

    TLGraphicTask job = {
        .type = TL_GRAPHICS_JOB_NO_ARGS,
        .func_no_args = func,
        .args = NULL,
        .is_sync = false,
        .completed = false,
        .completion_mutex = NULL,
        .completion_condition = NULL
    };

    if (!tl_graphics_queue_push(m_queue, &job)) {
        TLERROR("Failed to push asynchronous job to graphics queue")
    }

    TL_PROFILER_POP
}

void tl_graphics_submit_async_args(void (*func)(void*), void* args) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, args)

    if (!m_queue || !func) {
        TLERROR("Graphics queue not initialized or invalid function pointer")
        TL_PROFILER_POP
    }

    TLGraphicTask job = {
        .type = TL_GRAPHICS_JOB_WITH_ARGS,
        .func_with_args = func,
        .args = args,
        .is_sync = false,
        .completed = false,
        .completion_mutex = NULL,
        .completion_condition = NULL
    };

    if (!tl_graphics_queue_push(m_queue, &job)) {
        TLERROR("Failed to push asynchronous job with args to graphics queue")
    }

    TL_PROFILER_POP
}

// ---------------------------------
// Worker Thread
// ---------------------------------

static void* tl_graphics_worker(void* _) {
    TL_PROFILER_PUSH

    TLDEBUG("Initializing Graphics Context")
    glfwMakeContextCurrent(tl_window_handler());
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLFATAL("GLAD failed to initialize on graphics thread")
    }

    TLINFO("OpenGL %s", glGetString(GL_VERSION))
    TLDEBUG("CGLM %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    glfwSwapInterval(0);

    for ( ; ; ){
        tl_mutex_lock(m_queue->mutex);

        // Wait for work or shutdown signal
        // Thread blocks here when queue is empty, waiting for new work
        while (m_queue->count == 0 && !m_queue->shutdown) {
            tl_condition_wait(m_queue->not_empty, m_queue->mutex);
        }

        // Exit condition: shutdown requested and no pending work
        // This ensures all pending jobs are processed before termination
        if (m_queue->shutdown && m_queue->count == 0) {
            tl_mutex_unlock(m_queue->mutex);
            TLINFO("Shutdown requested, exiting worker loop")
            break;
        }

        // Pop job from queue
        TLGraphicTask job = m_queue->tasks[m_queue->tail];
        m_queue->tail = (m_queue->tail + 1) % m_queue->capacity;
        m_queue->count--;

        // Signal that space is available for producers
        tl_condition_signal(m_queue->not_full);
        tl_mutex_unlock(m_queue->mutex);

        // Execute job outside of lock to minimize contention
        if (job.type == TL_GRAPHICS_JOB_NO_ARGS) {
            TLVERBOSE("job.func_no_args()")
            job.func_no_args();
        } else {
            TLVERBOSE("job.func_with_args(job.args)")
            job.func_with_args(job.args);
        }

        // Signal completion for synchronous jobs
        if (job.is_sync) {
            tl_mutex_lock(job.completion_mutex);
            job.completed = true;
            tl_condition_signal(job.completion_condition);
            tl_mutex_unlock(job.completion_mutex);
        }
    }

    // Release OpenGL context
    glfwMakeContextCurrent(NULL);
    TLINFO("Graphics worker thread terminated")

    TL_PROFILER_POP_WITH(NULL)
}

// ---------------------------------
// Event Handlers
// ---------------------------------

static TLEventStatus tl_graphics_on_window_closed(const TLEvent* event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)

    if (!m_queue) TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)


    TLINFO("Window closed event received, shutting down graphics thread...")

    tl_mutex_lock(m_queue->mutex);
    m_queue->shutdown = true;
    tl_condition_broadcast(m_queue->not_empty);  // Wake up worker
    tl_condition_broadcast(m_queue->not_full);   // Wake up any waiting producers
    tl_mutex_unlock(m_queue->mutex);

    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

// ---------------------------------
// Lifecycle API
// ---------------------------------

b8 tl_graphics_initialize(void) {
    TL_PROFILER_PUSH

    // Subscribe to window closed event
    if (!tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, tl_graphics_on_window_closed)) {
        TLERROR("Failed to subscribe to window closed event")
        TL_PROFILER_POP_WITH(NULL)
    }

    m_allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4096), TL_ALLOCATOR_LINEAR);

    m_queue = tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, sizeof(TLGraphicsQueue));
    m_queue->tasks = tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, sizeof(TLGraphicTask) * TL_GRAPHICS_QUEUE_CAPACITY);
    m_queue->capacity = TL_GRAPHICS_QUEUE_CAPACITY;
    m_queue->mutex = tl_mutex_create();
    m_queue->not_empty = tl_condition_create();
    m_queue->not_full = tl_condition_create();

    if (!m_queue->mutex || !m_queue->not_empty || !m_queue->not_full) {\
        TLFATAL("Failed to create synchronization primitives")
    }

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

    // Signal shutdown (if not already done by event)
    tl_mutex_lock(m_queue->mutex);
    if (!m_queue->shutdown) {
        m_queue->shutdown = true;
        tl_condition_broadcast(m_queue->not_empty);
    }
    tl_mutex_unlock(m_queue->mutex);

    // Wait for worker thread to finish
    tl_thread_join(m_worker_thread, NULL);

    // Cleanup
    if (m_queue) {
        if (m_queue->mutex) tl_mutex_destroy(m_queue->mutex);
        if (m_queue->not_empty) tl_condition_destroy(m_queue->not_empty);
        if (m_queue->not_full) tl_condition_destroy(m_queue->not_full);
    }

    tl_memory_allocator_destroy(m_allocator);

    m_queue = NULL;
    m_worker_thread = NULL;
    m_allocator = NULL;

    TL_PROFILER_POP_WITH(true)
}
