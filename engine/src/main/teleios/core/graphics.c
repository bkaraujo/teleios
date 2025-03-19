#include "teleios/core.h"
#include "GLFW/glfw3.h"

b8 tl_graphics_initialize(void) {
    TLSTACKPUSH
    glfwMakeContextCurrent(runtime->platform.window.handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        TLSTACKPOP
        return FALSE;
    }

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

    TLSTACKPOP
    return TRUE;
}

b8 tl_graphics_terminate(void) {
    TLSTACKPUSH
    tl_memory_set(&runtime->engine.graphics, 0, sizeof(runtime->engine.graphics));
    TLSTACKPOP
    return TRUE;
}