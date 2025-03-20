#include "teleios/core.h"

b8 tl_graphics_initialize(void) {
    TLSTACKPUSH
    glfwMakeContextCurrent(runtime->platform.window.handle);

    TLDEBUG("CGLM_VERSION %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    TLTRACE("GL loading functions");
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        TLSTACKPOPV(FALSE)
    }

    TLDEBUG("GL_VERSION %s", glGetString(GL_VERSION))

    if (runtime->engine.graphics.vsync) {
        TLDEBUG("vsync: on")
        glfwSwapInterval(1);
    } else {
        TLDEBUG("vsync: off")
        glfwSwapInterval(0);
    }

    if (runtime->engine.graphics.wireframe) {
        TLDEBUG("wireframe: on")
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        TLDEBUG("wireframe: off")
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_graphics_terminate(void) {
    TLSTACKPUSH
    tl_memory_set(&runtime->engine.graphics, 0, sizeof(runtime->engine.graphics));
    TLSTACKPOPV(TRUE)
}