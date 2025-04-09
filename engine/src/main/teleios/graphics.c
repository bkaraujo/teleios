#include "teleios/core.h"
#include "teleios/globals.h"

b8 tl_graphics_initialize(void) {
    K_FRAME_PUSH

    KTRACE("Initializing Graphics Engine");
    glfwMakeContextCurrent(global->platform.window.handle);

    KDEBUG("CGLM_VERSION %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        KERROR("Failed to initialize GLAD")
        K_FRAME_POP_WITH(false)
    }

    KDEBUG("GL_VERSION %s", glGetString(GL_VERSION))

    if (global->platform.graphics.vsync) {
        KDEBUG("vsync: on")
        glfwSwapInterval(1);
    } else {
        KDEBUG("vsync: off")
        glfwSwapInterval(0);
    }

    if (global->platform.graphics.wireframe) {
        KDEBUG("wireframe: on")
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        KDEBUG("wireframe: off")
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    K_FRAME_POP_WITH(true)
}

b8 tl_graphics_terminate(void) {
    K_FRAME_PUSH
    KTRACE("Terminating Graphics Engine");
    k_memory_set(&global->platform.graphics, 0, sizeof(global->platform.graphics));
    glfwMakeContextCurrent(NULL);
    K_FRAME_POP_WITH(true)
}