#ifndef __TELEIOS_GRAPHICS_QUEUE__
#define __TELEIOS_GRAPHICS_QUEUE__

#include "memory_dynamic.inl"
#include "teleios/teleios.h"
#include "teleios/graphics_types.inl"

// Forward declarations from graphics.c
extern TLAllocator* m_allocator;

// ---------------------------------
// Graphics Submit - Internal Implementation
// ---------------------------------

static void* tl_graphics_submit(TLGraphicTask* task) {
    TL_PROFILER_PUSH_WITH("0x%p, is_sync=%d", task, task->is_sync)

    task->completed = false;
    task->result = NULL;

    if (!task->is_sync) {
        task->heap_allocated = true;            // Asynchronous job: allocate on heap (worker will free it)
        task->completion_mutex = NULL;
        task->completion_condition = NULL;

        TLTRACE("Async task submitted: 0x%p", task)
        tl_queue_push(m_queue, task);           // Push to queue (returns immediately, no waiting)

        TL_PROFILER_POP_WITH(NULL)
    }

    // Synchronous job: create synchronization primitives and wait
    task->completion_mutex = tl_mutex_create(m_allocator);
    task->completion_condition = tl_condition_create(m_allocator);

    if (!task->completion_mutex || !task->completion_condition) {
        TLFATAL("Failed to create synchronization primitives for sync graphics task")
    }

    TLTRACE("Sync task submitted: 0x%p", task)
    tl_mutex_lock(task->completion_mutex); // Lock mutex BEFORE pushing to queue (prevents lost wakeup)
    tl_queue_push(m_queue, task); // Push to queue (queue has its own internal lock)

    // Wait for completion (atomically unlocks mutex and waits, then relocks on wakeup)
    while (!task->completed) {
        tl_condition_wait(task->completion_condition, task->completion_mutex);
    }

    tl_mutex_unlock(task->completion_mutex);    // Unlock mutex after waking up
    void* result = task->result; // Store result before destroying sync primitives

    tl_mutex_destroy(task->completion_mutex);   // Cleanup synchronization primitives
    tl_condition_destroy(task->completion_condition);

    if (task->heap_allocated) {
        tl_memory_free(m_allocator, task);
    }

    TL_PROFILER_POP_WITH(result)
}

// ---------------------------------
// Public API - Graphics Submission Functions
// ---------------------------------

void* tl_graphics_submit_sync(void* (*func)(void)) {
    TL_PROFILER_PUSH_WITH("0x%p", func)

    // Stack allocation is safe for sync jobs (thread waits until completion)
    TLGraphicTask job = {
        .type = TL_GRAPHICS_JOB_NO_ARGS,
        .func_no_args = func,
        .args = NULL,
        .is_sync = true,
        .heap_allocated = false
    };

    void* result = tl_graphics_submit(&job);
    TL_PROFILER_POP_WITH(result)
}

void* tl_graphics_submit_sync_args(void* (*func)(void*), void* args) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, args)

    // Stack allocation is safe for sync jobs
    TLGraphicTask job = {
        .type = TL_GRAPHICS_JOB_WITH_ARGS,
        .func_with_args = func,
        .args = args,
        .is_sync = true,
        .heap_allocated = false
    };

    void* result = tl_graphics_submit(&job);
    TL_PROFILER_POP_WITH(result)
}

void* tl_graphics_submit_async(void* (*func)(void)) {
    TL_PROFILER_PUSH_WITH("0x%p", func)

    // Heap allocation required for async jobs (stack will be invalid when worker accesses)
    TLGraphicTask* job = tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, sizeof(TLGraphicTask));
    job->type = TL_GRAPHICS_JOB_NO_ARGS;
    job->func_no_args = func;
    job->args = NULL;
    job->is_sync = false;
    job->heap_allocated = true;

    // Heap deallocation after consumed
    void* result = tl_graphics_submit(job);
    TL_PROFILER_POP_WITH(result)
}

void* tl_graphics_submit_async_args(void* (*func)(void*), void* args) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, args)

    // Heap allocation required for async jobs
    TLGraphicTask* job = tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, sizeof(TLGraphicTask));
    job->type = TL_GRAPHICS_JOB_WITH_ARGS;
    job->func_with_args = func;
    job->args = args;
    job->is_sync = false;
    job->heap_allocated = true;

    // Heap deallocation after consumed
    void* result = tl_graphics_submit(job);
    TL_PROFILER_POP_WITH(result)
}

#endif
