#include "teleios/core.h"
#include "teleios/globals.h"

b8 tl_graphics_initialize(void) {
    BKS_TRACE_PUSH

    BKSTRACE("Initializing Graphics Engine");
    glfwMakeContextCurrent(global->platform.window.handle);

    BKSDEBUG("CGLM_VERSION %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        BKSERROR("Failed to initialize GLAD")
        BKS_TRACE_POPV(false)
    }

    BKSDEBUG("GL_VERSION %s", glGetString(GL_VERSION))

    if (global->platform.graphics.vsync) {
        BKSDEBUG("vsync: on")
        glfwSwapInterval(1);
    } else {
        BKSDEBUG("vsync: off")
        glfwSwapInterval(0);
    }

    if (global->platform.graphics.wireframe) {
        BKSDEBUG("wireframe: on")
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        BKSDEBUG("wireframe: off")
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    BKS_TRACE_POPV(true)
}

b8 tl_graphics_terminate(void) {
    BKS_TRACE_PUSH
    BKSTRACE("Terminating Graphics Engine");
    tl_platform_memory_set(&global->platform.graphics, 0, sizeof(global->platform.graphics));
    glfwMakeContextCurrent(NULL);
    BKS_TRACE_POPV(true)
}