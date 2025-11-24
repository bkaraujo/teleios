#ifndef __TELEIOS_GRAPHICS_CMDBUFFER__
#define __TELEIOS_GRAPHICS_CMDBUFFER__

#include "teleios/teleios.h"

// ---------------------------------
// Command Buffer Pool State
// ---------------------------------

static TLCommandBufferPool* m_cmdbuffer_pool = NULL;

// ---------------------------------
// Internal Functions
// ---------------------------------

static void tl_cmdbuffer_pool_init(void) {
    TL_PROFILER_PUSH

    m_cmdbuffer_pool = tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, sizeof(TLCommandBufferPool));
    if (!m_cmdbuffer_pool) {
        TLFATAL("Failed to allocate command buffer pool")
    }

    m_cmdbuffer_pool->allocator = m_allocator;
    m_cmdbuffer_pool->recording_index = 0xFF;  // No buffer being recorded

    // Initialize all buffers as FREE
    for (u8 i = 0; i < TL_CMDBUFFER_POOL_SIZE; ++i) {
        m_cmdbuffer_pool->buffers[i].count = 0;
        m_cmdbuffer_pool->buffers[i].state = TL_CMDBUFFER_FREE;
    }

    m_cmdbuffer_pool->mutex = tl_mutex_create(m_allocator);
    m_cmdbuffer_pool->buffer_available = tl_condition_create(m_allocator);
    m_cmdbuffer_pool->buffer_pending = tl_condition_create(m_allocator);

    if (!m_cmdbuffer_pool->mutex || !m_cmdbuffer_pool->buffer_available || !m_cmdbuffer_pool->buffer_pending) {
        TLFATAL("Failed to create synchronization primitives for command buffer pool")
    }

    TLDEBUG("Command buffer pool initialized: %u buffers, %u commands each",
        TL_CMDBUFFER_POOL_SIZE, TL_CMDBUFFER_CAPACITY)

    TL_PROFILER_POP
}

static void tl_cmdbuffer_pool_destroy(void) {
    TL_PROFILER_PUSH

    if (!m_cmdbuffer_pool) {
        TL_PROFILER_POP
    }

    // Free any heap-allocated args in pending commands
    for (u8 i = 0; i < TL_CMDBUFFER_POOL_SIZE; ++i) {
        TLCommandBuffer* buffer = &m_cmdbuffer_pool->buffers[i];
        for (u32 j = 0; j < buffer->count; ++j) {
            if (buffer->commands[j].args) {
                tl_memory_free(m_allocator, buffer->commands[j].args);
            }
        }
    }

    if (m_cmdbuffer_pool->buffer_pending) {
        tl_condition_destroy(m_cmdbuffer_pool->buffer_pending);
    }
    if (m_cmdbuffer_pool->buffer_available) {
        tl_condition_destroy(m_cmdbuffer_pool->buffer_available);
    }
    if (m_cmdbuffer_pool->mutex) {
        tl_mutex_destroy(m_cmdbuffer_pool->mutex);
    }

    tl_memory_free(m_allocator, m_cmdbuffer_pool);
    m_cmdbuffer_pool = NULL;

    TL_PROFILER_POP
}

// Find and acquire a FREE buffer, blocking if none available
static u8 tl_cmdbuffer_acquire_free(void) {
    TL_PROFILER_PUSH

    tl_mutex_lock(m_cmdbuffer_pool->mutex);

    // Find a FREE buffer
    while (true) {
        for (u8 i = 0; i < TL_CMDBUFFER_POOL_SIZE; ++i) {
            if (m_cmdbuffer_pool->buffers[i].state == TL_CMDBUFFER_FREE) {
                m_cmdbuffer_pool->buffers[i].state = TL_CMDBUFFER_RECORDING;
                m_cmdbuffer_pool->buffers[i].count = 0;
                m_cmdbuffer_pool->recording_index = i;
                tl_mutex_unlock(m_cmdbuffer_pool->mutex);
                TL_PROFILER_POP_WITH(i)
            }
        }

        // No free buffer available, wait for one to be released
        tl_condition_wait(m_cmdbuffer_pool->buffer_available, m_cmdbuffer_pool->mutex);
    }
}

// Get the next PENDING buffer for execution (non-blocking)
// Returns NULL if no buffer is pending
static TLCommandBuffer* tl_cmdbuffer_acquire_pending(void) {
    TL_PROFILER_PUSH

    tl_mutex_lock(m_cmdbuffer_pool->mutex);

    for (u8 i = 0; i < TL_CMDBUFFER_POOL_SIZE; ++i) {
        if (m_cmdbuffer_pool->buffers[i].state == TL_CMDBUFFER_PENDING) {
            m_cmdbuffer_pool->buffers[i].state = TL_CMDBUFFER_EXECUTING;
            tl_mutex_unlock(m_cmdbuffer_pool->mutex);
            TL_PROFILER_POP_WITH(&m_cmdbuffer_pool->buffers[i])
        }
    }

    tl_mutex_unlock(m_cmdbuffer_pool->mutex);
    TL_PROFILER_POP_WITH(NULL)
}

// Wait for a pending buffer or shutdown signal
static void tl_cmdbuffer_wait_for_work(void) {
    tl_mutex_lock(m_cmdbuffer_pool->mutex);

    // Check if there's any pending buffer
    b8 has_pending = false;
    for (u8 i = 0; i < TL_CMDBUFFER_POOL_SIZE; ++i) {
        if (m_cmdbuffer_pool->buffers[i].state == TL_CMDBUFFER_PENDING) {
            has_pending = true;
            break;
        }
    }

    // If no pending and not shutdown, wait with timeout
    if (!has_pending && !m_shutdown) {
        // Use timed wait to avoid deadlock - check every 1ms
        tl_condition_wait_timeout(m_cmdbuffer_pool->buffer_pending, m_cmdbuffer_pool->mutex, 1);
    }

    tl_mutex_unlock(m_cmdbuffer_pool->mutex);
}

// Mark buffer as FREE after execution
static void tl_cmdbuffer_release(TLCommandBuffer* buffer) {
    TL_PROFILER_PUSH

    tl_mutex_lock(m_cmdbuffer_pool->mutex);

    // Free heap-allocated args
    for (u32 i = 0; i < buffer->count; ++i) {
        if (buffer->commands[i].args) {
            tl_memory_free(m_allocator, buffer->commands[i].args);
            buffer->commands[i].args = NULL;
        }
    }

    buffer->count = 0;
    buffer->state = TL_CMDBUFFER_FREE;

    tl_condition_signal(m_cmdbuffer_pool->buffer_available);
    tl_mutex_unlock(m_cmdbuffer_pool->mutex);

    TL_PROFILER_POP
}

// Record a command to the current buffer
void tl_cmdbuffer_record(void* (*func_no_args)(void), void* (*func_with_args)(void**), void** args, u32 args_count) {
    TL_PROFILER_PUSH

    if (m_cmdbuffer_pool->recording_index == 0xFF) {
        TLFATAL("No command buffer is being recorded. Call tl_cmdbuffer_begin() first.")
    }

    TLCommandBuffer* buffer = &m_cmdbuffer_pool->buffers[m_cmdbuffer_pool->recording_index];

    if (buffer->count >= TL_CMDBUFFER_CAPACITY) {
        TLFATAL("Command buffer overflow: max %u commands", TL_CMDBUFFER_CAPACITY)
    }

    TLCommand* cmd = &buffer->commands[buffer->count++];

    if (func_no_args) {
        cmd->func_no_args = func_no_args;
        cmd->type = TL_GRAPHICS_JOB_NO_ARGS;
        cmd->args = NULL;
        cmd->args_count = 0;
    } else {
        cmd->func_with_args = func_with_args;
        cmd->type = TL_GRAPHICS_JOB_WITH_ARGS;

        // Copy args to heap (they may be stack-allocated by caller)
        if (args_count > 0 && args) {
            cmd->args = tl_memory_alloc(m_allocator, TL_MEMORY_GRAPHICS, sizeof(void*) * args_count);
            tl_memory_copy(cmd->args, args, sizeof(void*) * args_count);
            cmd->args_count = args_count;
        } else {
            cmd->args = NULL;
            cmd->args_count = 0;
        }
    }

    TL_PROFILER_POP
}

// Execute all commands in a buffer
static void tl_cmdbuffer_execute(TLCommandBuffer* buffer) {
    TL_PROFILER_PUSH

    for (u32 i = 0; i < buffer->count; ++i) {
        TLCommand* cmd = &buffer->commands[i];

        if (cmd->type == TL_GRAPHICS_JOB_NO_ARGS) {
            cmd->func_no_args();
        } else {
            cmd->func_with_args(cmd->args);
        }
    }

    TL_PROFILER_POP
}

// ---------------------------------
// Public API
// ---------------------------------

void tl_cmdbuffer_begin(void) {
    TL_PROFILER_PUSH

    if (m_cmdbuffer_pool->recording_index != 0xFF) {
        TLFATAL("Already recording a command buffer. Call tl_cmdbuffer_end() first.")
    }

    tl_cmdbuffer_acquire_free();

    TL_PROFILER_POP
}

void tl_cmdbuffer_end(void) {
    TL_PROFILER_PUSH

    if (m_cmdbuffer_pool->recording_index == 0xFF) {
        TLFATAL("No command buffer is being recorded. Call tl_cmdbuffer_begin() first.")
    }

    tl_mutex_lock(m_cmdbuffer_pool->mutex);

    TLCommandBuffer* buffer = &m_cmdbuffer_pool->buffers[m_cmdbuffer_pool->recording_index];
    buffer->state = TL_CMDBUFFER_PENDING;
    m_cmdbuffer_pool->recording_index = 0xFF;

    tl_condition_signal(m_cmdbuffer_pool->buffer_pending);
    tl_mutex_unlock(m_cmdbuffer_pool->mutex);

    TL_PROFILER_POP
}

void tl_cmdbuffer_sync(void) {
    TL_PROFILER_PUSH

    tl_mutex_lock(m_cmdbuffer_pool->mutex);

    // Wait until all buffers are FREE (none pending or executing)
    while (true) {
        b8 all_free = true;
        for (u8 i = 0; i < TL_CMDBUFFER_POOL_SIZE; ++i) {
            TLCommandBufferState state = m_cmdbuffer_pool->buffers[i].state;
            if (state == TL_CMDBUFFER_PENDING || state == TL_CMDBUFFER_EXECUTING) {
                all_free = false;
                break;
            }
        }

        if (all_free) {
            break;
        }

        tl_condition_wait(m_cmdbuffer_pool->buffer_available, m_cmdbuffer_pool->mutex);
    }

    tl_mutex_unlock(m_cmdbuffer_pool->mutex);

    TL_PROFILER_POP
}

#endif
