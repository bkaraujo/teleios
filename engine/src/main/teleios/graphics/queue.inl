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

TLGraphicsTask* tl_graphics_acquire_task(const b8 wait) {
    TLGraphicsTask* task = tl_pool_acquire_wait(m_pool);
    task->type = TL_TASK_UNKNOWN;
    task->wait = wait;
    task->function.raw = NULL;
    task->is_complete = false;
    task->result = NULL;
    task->argc = 0;
    task->argv = NULL;

    return task;
}

void* tl_graphics_submit_rna(const b8 wait, const TLFunctionRNA function) {
    TLGraphicsTask* task = tl_graphics_acquire_task(wait);
    task->type = TL_RETURN_WITH_NO_ARG;
    task->function.rna = function;
    return tl_graphics_submit(task);
}

void tl_graphics_submit_vna(const b8 wait, const TLFunctionVNA function) {
    TLGraphicsTask* task = tl_graphics_acquire_task(wait);

    task->type = TL_NORETURN_WITH_NO_ARG;
    task->function.vna = function;
    tl_graphics_submit(task);
}

void* tl_graphics_submit_rwa(const b8 wait, const TLFunctionRWA function, const u8 argc, void** argv) {
    TLGraphicsTask* task = tl_graphics_acquire_task(wait);

    task->type = TL_RETURN_WITH_ARG;
    task->argv = argv;
    task->argc = argc;
    task->function.rwa = function;
    return tl_graphics_submit(task);
}

void tl_graphics_submit_vwa(const b8 wait, const TLFunctionVWA function, const u8 argc, void** argv) {
    TLGraphicsTask* task = tl_graphics_acquire_task(wait);

    task->type = TL_NORETURN_WITH_ARG;
    task->argv = argv;
    task->argc = argc;
    task->function.vwa = function;
    tl_graphics_submit(task);
}

#endif