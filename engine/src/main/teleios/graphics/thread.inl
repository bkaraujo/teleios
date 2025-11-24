#ifndef __TELEIOS_GRAPHICS_THREAD__
#define __TELEIOS_GRAPHICS_THREAD__

#include "teleios/graphics/types.inl"
#include "teleios/teleios.h"
#include "teleios/graphics/queue.inl"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/version.h>

// ---------------------------------
// Graphics Worker Thread
// ---------------------------------

static void* tl_graphics_worker(void* _) {
    (void) _;  // Unused parameter
    TL_PROFILER_PUSH

    m_thread_id = tl_thread_id();

    // #########################################
    // Initialize OpenGL context
    // #########################################
    glfwMakeContextCurrent(tl_window_handler());
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLFATAL("GLAD failed to initialize on graphics thread")
    }

    TLDEBUG("OpenGL %s", glGetString(GL_VERSION))
    TLDEBUG("CGLM %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)
    // #########################################
    // Process global application.yaml options
    // #########################################
    const b8 vsync = tl_config_get_b8("teleios.graphics.vsync");
    glfwSwapInterval(vsync ? 1 : 0);

    const b8 wireframe = tl_config_get_b8("teleios.graphics.wireframe");
    if (wireframe)  { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    else            { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

    glClearColor(0.126f, 0.48f, 1.0f, 1.0f);
    // #########################################
    // Main worker loop (Command Buffer Mode)
    // #########################################
    TLDEBUG("Graphics worker entering main loop")
    for ( ; ; ) {

        // Check shutdown flag before processing
        if (m_shutdown) {
            TLDEBUG("Graphics worker received shutdown signal")
            break;
        }

        // Try to get a pending command buffer (non-blocking)
        TLCommandBuffer* cmdbuffer = tl_cmdbuffer_acquire_pending();
        if (cmdbuffer != NULL) {
            tl_cmdbuffer_execute(cmdbuffer);
            tl_cmdbuffer_release(cmdbuffer);
            continue;
        }

        // No command buffer, check for sync jobs in the queue (non-blocking)
        TLGraphicTask* task = (TLGraphicTask*)tl_queue_pop(m_queue);
        if (task != NULL) {
            tl_graphics_execute(task);
            continue;
        }

        // Nothing to do, wait for work (with timeout to check shutdown)
        tl_cmdbuffer_wait_for_work();
    }

    // Release OpenGL context before exiting
    glfwMakeContextCurrent(NULL);
    TLDEBUG("Graphics thread exiting")

    TL_PROFILER_POP_WITH(NULL)
}

#endif
