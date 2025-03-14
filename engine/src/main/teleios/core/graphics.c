#include "teleios/core/graphics.h"
#include "teleios/core/memory.h"
#include "teleios/core/logger.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

b8 tl_graphics_initialize(void) {
    TLTRACE(">> tl_graphics_initialize(void)")
    glfwMakeContextCurrent(runtime->platform.window.handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        return FALSE;
    }

    if (runtime->graphics.vsync) {
        TLVERBOSE("vsync: on")
        glfwSwapInterval(1);
    } else {
        TLVERBOSE("vsync: off")
        glfwSwapInterval(0);
    }

    if (runtime->graphics.wireframe) {
        TLVERBOSE("wireframe: on")
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        TLVERBOSE("wireframe: off")
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    TLTRACE(">> tl_graphics_initialize(void)")
    return TRUE;
}

b8 tl_graphics_terminate(void) {
    TLTRACE(">> tl_graphics_terminate(void)")
    tl_memory_set(&runtime->graphics, 0, sizeof(runtime->graphics));
    TLTRACE(">> tl_graphics_terminate(void)")
    return TRUE;
}