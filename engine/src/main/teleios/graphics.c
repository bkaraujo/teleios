#include "teleios/core.h"
#include "teleios/globals.h"

b8 tl_graphics_initialize(void) {
    TL_STACK_PUSH

    TLTRACE("Initializing Graphics Engine");
    glfwMakeContextCurrent(global->platform.window.handle);

    TLDEBUG("CGLM_VERSION %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        TL_STACK_POPV(false)
    }

    TLDEBUG("GL_VERSION %s", glGetString(GL_VERSION))

    if (global->platform.graphics.vsync) {
        TLDEBUG("vsync: on")
        glfwSwapInterval(1);
    } else {
        TLDEBUG("vsync: off")
        glfwSwapInterval(0);
    }

    if (global->platform.graphics.wireframe) {
        TLDEBUG("wireframe: on")
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        TLDEBUG("wireframe: off")
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    TL_STACK_POPV(true)
}

b8 tl_graphics_terminate(void) {
    TL_STACK_PUSH
    TLTRACE("Terminating Graphics Engine");
    tl_platform_memory_set(&global->platform.graphics, 0, sizeof(global->platform.graphics));
    glfwMakeContextCurrent(NULL);
    TL_STACK_POPV(true)
}