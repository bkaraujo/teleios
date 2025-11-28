#ifndef __TELEIOS_GRAPHICS_THREAD__
#define __TELEIOS_GRAPHICS_THREAD__

#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/version.h>

static void* tl_graphics_thread(void* _) {
    (void) _;
    TLDEBUG("Initializing")
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

    TLDEBUG("Entering Main Loop")
    for ( ; global->running ; ) {
        TLGraphicsTask* task = NULL;
        while ((task = (TLGraphicsTask*) tl_queue_pop(m_queue)) != NULL) {

            switch (task->type) {
                case TL_RETURN_WITH_NO_ARG: {
                    task->result = task->function.rna();
                } break;

                case TL_RETURN_WITH_ARG: {
                    task->result = task->function.rwa(task->argc, task->argv);
                } break;

                case TL_NORETURN_WITH_ARG: {
                    task->function.vwa(task->argc, task->argv);
                } break;

                case TL_NORETURN_WITH_NO_ARG: {
                    task->function.vna();
                } break;
            }

            if (task->wait) {
                task->is_complete = true;
                tl_mutex_lock(task->mutex);
                tl_condition_signal(task->condition);
                tl_mutex_unlock(task->mutex);

                // caller releases task into pool
                // caller need the result
                continue;
            }

            // Release async task
            tl_pool_release(m_pool, task);
        }
    }
    TLDEBUG("Exiting Main Loop")

    glfwMakeContextCurrent(NULL);
    TLDEBUG("Finalizando")

    return NULL;
}

#endif