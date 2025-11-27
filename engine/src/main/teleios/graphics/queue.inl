#ifndef __TELEIOS_GRAPHICS_QUEUE__
#define __TELEIOS_GRAPHICS_QUEUE__

#include "teleios/teleios.h"

static void* tl_graphics_submit(TLGraphicsTask* task) {
    if (tl_thread_current_id() == tl_thread_id(m_thread)) {
        TLFATAL("Cannot call tl_graphics_submit from graphics thread itself")
    }

    tl_queue_push(m_queue, task);

    if (task->wait) {
        tl_mutex_lock(task->mutex);
        while (!task->is_complete) {
            tl_condition_wait(task->condition, task->mutex);
        }
        tl_mutex_unlock(task->mutex);

        void* result = task->result;
        tl_pool_release(m_pool, task);

        return result;
    }

    return NULL;
}

void* tl_graphics_submit_rna(b8 wait, TLFunctionRNA function) {
    TLGraphicsTask* task = tl_pool_acquire_wait(m_pool);
    task->type = TL_RETURN_WITH_NO_ARG;
    task->wait = wait;
    task->function.rna = function;
    return tl_graphics_submit(task);
}

void tl_graphics_submit_vna(b8 wait, TLFunctionVNA function) {
    TLGraphicsTask* task = tl_pool_acquire_wait(m_pool);

    task->type = TL_NORETURN_WITH_NO_ARG;
    task->wait = wait;
    task->function.vna = function;
    tl_graphics_submit(task);
}

void* tl_graphics_submit_rwa(b8 wait, TLFunctionRWA function, u8 argc, void* argv) {
    TLGraphicsTask* task = tl_pool_acquire_wait(m_pool);

    task->type = TL_RETURN_WITH_ARG;
    task->wait = wait;
    task->argv = argv;
    task->argc = argc;
    task->function.rwa = function;
    return tl_graphics_submit(task);
}

void tl_graphics_submit_vwa(b8 wait, TLFunctionVWA function, u8 argc, void* argv) {
    TLGraphicsTask* task = tl_pool_acquire_wait(m_pool);

    task->type = TL_NORETURN_WITH_ARG;
    task->wait = wait;
    task->argv = argv;
    task->argc = argc;
    task->function.vwa = function;
    tl_graphics_submit(task);
}

#endif