#ifndef __TELEIOS_GRAPHICS_QUEUE__
#define __TELEIOS_GRAPHICS_QUEUE__

#include "teleios/graphics/types.inl"
#include "teleios/memory/dynamic.inl"
#include "teleios/teleios.h"

// ---------------------------------
// Graphics Submit - Internal Implementation
// ---------------------------------

static void tl_graphics_execute(TLGraphicTask* task) {
    // Execute the task based on type
    if (task->type == TL_GRAPHICS_JOB_NO_ARGS) {
        task->result = task->func_no_args();
    } else {
        task->result = task->func_with_args(task->args);
    }

    // Handle completion signaling for sync jobs
    if (task->is_sync) {
        // Lock mutex, set completed flag, signal waiting thread
        tl_mutex_lock(task->completion_mutex);
        task->completed = true;
        tl_condition_signal(task->completion_condition);
        tl_mutex_unlock(task->completion_mutex);
        // Note: Sync jobs are released back to pool by the submitting thread
        // Note: Args for sync jobs are stack-allocated by caller, no need to free
    } else {
        // Async jobs: free heap-allocated args array if present
        if (task->args != NULL) {
            tl_memory_free(m_allocator, task->args);
            task->args = NULL;
        }
        // Async jobs: release back to pool for reuse
        tl_pool_release(m_task_pool, task);
    }
}

static void* tl_graphics_submit(TLGraphicTask* task) {
    TL_PROFILER_PUSH_WITH("0x%p, is_sync=%d", task, task->is_sync)

    if (tl_graphics_is_thread()) {
        tl_graphics_execute(task);
        return task->result;
    }

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
    TLGraphicTask* task = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!task) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Initialize task
    task->type = TL_GRAPHICS_JOB_NO_ARGS;
    task->func_no_args = func;
    task->args = NULL;
    task->args_count = 0;
    task->is_sync = true;

    void* result = tl_graphics_submit(task);
    TL_PROFILER_POP_WITH(result)
}

void* _tl_graphics_submit_sync_args(void* (*func)(void**), u32 count, ...) {
    TL_PROFILER_PUSH_WITH("0x%p, count=%u", func, count)

    // Acquire task from pool
    TLGraphicTask* task = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!task) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Stack-allocate argument array (safe because sync call blocks until completion)
    void* arg_array[16];  // Max 16 arguments
    if (count > 16) {
        TLFATAL("Too many arguments for graphics job (max 16, got %u)", count)
    }

    // Pack varargs into array
    va_list args;
    va_start(args, count);
    for (u32 i = 0; i < count; i++) {
        arg_array[i] = va_arg(args, void*);
    }
    va_end(args);

    // Initialize task
    task->type = TL_GRAPHICS_JOB_WITH_ARGS;
    task->func_with_args = func;
    task->args = arg_array;
    task->args_count = count;
    task->is_sync = true;

    void* result = tl_graphics_submit(task);
    TL_PROFILER_POP_WITH(result)
}

void* tl_graphics_submit_async(void* (*func)(void)) {
    TL_PROFILER_PUSH_WITH("0x%p", func)

    // Acquire task from pool
    TLGraphicTask* task = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!task) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Initialize task
    task->type = TL_GRAPHICS_JOB_NO_ARGS;
    task->func_no_args = func;
    task->args = NULL;
    task->args_count = 0;
    task->is_sync = false;

    void* result = tl_graphics_submit(task);
    TL_PROFILER_POP_WITH(result)
}

void* _tl_graphics_submit_async_args(void* (*func)(void**), u32 count, ...) {
    TL_PROFILER_PUSH_WITH("0x%p, count=%u", func, count)

    // Acquire task from pool
    TLGraphicTask* task = (TLGraphicTask*)tl_pool_acquire(m_task_pool);
    if (!task) {
        TLFATAL("Graphics task pool exhausted (capacity=%u)", tl_pool_capacity(m_task_pool))
    }

    // Heap-allocate argument array (required because async call returns immediately)
    if (count > 16) {
        TLFATAL("Too many arguments for graphics job (max 16, got %u)", count)
    }

    void** arg_array = (void**)tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, sizeof(void*) * count);
    if (!arg_array) {
        TLFATAL("Failed to allocate argument array for async graphics job")
    }

    // Pack varargs into heap-allocated array
    va_list args;
    va_start(args, count);
    for (u32 i = 0; i < count; i++) {
        arg_array[i] = va_arg(args, void*);
    }
    va_end(args);

    // Initialize task
    task->type = TL_GRAPHICS_JOB_WITH_ARGS;
    task->func_with_args = func;
    task->args = arg_array;
    task->args_count = count;
    task->is_sync = false;

    void* result = tl_graphics_submit(task);
    TL_PROFILER_POP_WITH(result)
}

#endif
