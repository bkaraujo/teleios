#ifndef __TELEIOS_GRAPHICS_THREAD__
#define __TELEIOS_GRAPHICS_THREAD__

#include "teleios/graphics/types.inl"
#include "teleios/teleios.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/version.h>

// ---------------------------------
// Graphics Worker Thread
// ---------------------------------

static void* tl_graphics_worker(void* _) {
    (void) _;  // Unused parameter
    TL_PROFILER_PUSH
    TLDEBUG("Graphics thread initializing")

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

        // Check shutdown flag before blocking on queue
        if (m_shutdown) {
            TLDEBUG("Graphics worker received shutdown signal")
            break;
        }

        // Pop task from queue (blocks if empty)
        TLGraphicTask* task = (TLGraphicTask*)tl_queue_pop(m_queue);
        if (task == NULL) { continue; }

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
    TLDEBUG("Graphics thread exiting")

    TL_PROFILER_POP_WITH(NULL)
}

#endif
