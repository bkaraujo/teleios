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

    glfwShowWindow(tl_window_handler());

    TLDEBUG("Entering Main Loop")
    for ( ; global->running ; ) {
        global->frame_count++;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // tl_command_execute()

        glfwSwapBuffers(tl_window_handler());
    }
    TLDEBUG("Exiting Main Loop")

    glfwMakeContextCurrent(NULL);
    TLDEBUG("Finalizando")

    return NULL;
}

#endif