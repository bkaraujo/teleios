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
        // Asynchronous job: just push to queue (worker will release back to pool)
        TLTRACE("Async task submitted: 0x%p", task)
        tl_queue_push(m_queue, task);
        TL_PROFILER_POP_WITH(NULL)
    }

    // Synchronous job: use pre-allocated synchronization primitives and wait
    TLTRACE("Sync task submitted: 0x%p", task)
    tl_mutex_lock(task->completion_mutex);  // Lock mutex BEFORE pushing to queue (prevents lost wakeup)
    tl_queue_push(m_queue, task);            // Push to queue (queue has its own internal lock)

    // Wait for completion (atomically unlocks mutex and waits, then relocks on wakeup)
    while (!task->completed) {
        tl_condition_wait(task->completion_condition, task->completion_mutex);
    }

    tl_mutex_unlock(task->completion_mutex);    // Unlock mutex after waking up
    void* result = task->result;                // Store result before releasing task

    // Release task back to pool for reuse
    tl_pool_release(m_task_pool, task);

    TL_PROFILER_POP_WITH(result)
}

// ---------------------------------
// Public API - Graphics Submission Functions
// ---------------------------------

void* tl_graphics_submit_sync(void* (*func)(void)) {
    TL_PROFILER_PUSH_WITH("0x%p", func)

    // Acquire task from pool
    TLGraphicTask* job = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!job) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Initialize task
    job->type = TL_GRAPHICS_JOB_NO_ARGS;
    job->func_no_args = func;
    job->args = NULL;
    job->is_sync = true;

    void* result = tl_graphics_submit(job);
    TL_PROFILER_POP_WITH(result)
}

void* tl_graphics_submit_sync_args(void* (*func)(void*), void* args) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, args)

    // Acquire task from pool
    TLGraphicTask* job = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!job) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Initialize task
    job->type = TL_GRAPHICS_JOB_WITH_ARGS;
    job->func_with_args = func;
    job->args = args;
    job->is_sync = true;

    void* result = tl_graphics_submit(job);
    TL_PROFILER_POP_WITH(result)
}

void* tl_graphics_submit_async(void* (*func)(void)) {
    TL_PROFILER_PUSH_WITH("0x%p", func)

    // Acquire task from pool
    TLGraphicTask* job = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!job) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Initialize task
    job->type = TL_GRAPHICS_JOB_NO_ARGS;
    job->func_no_args = func;
    job->args = NULL;
    job->is_sync = false;

    void* result = tl_graphics_submit(job);
    TL_PROFILER_POP_WITH(result)
}

void* tl_graphics_submit_async_args(void* (*func)(void*), void* args) {
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p", func, args)

    // Acquire task from pool
    TLGraphicTask* job = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!job) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Initialize task
    job->type = TL_GRAPHICS_JOB_WITH_ARGS;
    job->func_with_args = func;
    job->args = args;
    job->is_sync = false;

    void* result = tl_graphics_submit(job);
    TL_PROFILER_POP_WITH(result)
}

#endif
