#ifndef __TELEIOS_GRAPHICS_THREAD__
#define __TELEIOS_GRAPHICS_THREAD__

#include "teleios/teleios.h"
#include "teleios/graphics_types.inl"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/version.h>

// ---------------------------------
// Graphics Worker Thread
// ---------------------------------

static void* tl_graphics_worker(void* _) {
    (void) _;  // Unused parameter
    TL_PROFILER_PUSH

    // Acquire OpenGL context on this thread
    glfwMakeContextCurrent(tl_window_handler());
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLFATAL("GLAD failed to initialize on graphics thread")
    }

    TLDEBUG("OpenGL %s", glGetString(GL_VERSION))
    TLDEBUG("CGLM %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    // Disable VSync for immediate buffer swaps
    glfwSwapInterval(0);
    glClearColor(0.126f, 0.48f, 1.0f, 1.0f);

    // Main worker loop - process tasks from queue until shutdown signal
    for ( ; ; ) {

        // Pop task from queue (blocks if empty)
        TLGraphicTask* task = (TLGraphicTask*)tl_queue_pop(m_queue);

        if (task == NULL) {
            TLWARN("Graphics worker received NULL task, continuing...")
            continue;
        }

        // Check for shutdown signal (both function pointers NULL)
        if (task->func_no_args == NULL && task->func_with_args == NULL) {
            TLDEBUG("Graphics worker received shutdown signal")
            break;
        }

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
        } else {
            // Async jobs: release back to pool for reuse
            tl_pool_release(m_task_pool, task);
        }
    }

    // Release OpenGL context before exiting
    glfwMakeContextCurrent(NULL);
    TLDEBUG("Graphics worker thread exiting")

    TL_PROFILER_POP_WITH(NULL)
}

#endif
