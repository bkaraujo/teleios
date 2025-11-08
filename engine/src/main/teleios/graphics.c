#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/version.h>

b8 tl_graphics_initialize(void) {
    TL_PROFILER_PUSH

    glfwMakeContextCurrent(tl_window_handler());
    TLDEBUG("CGLM v%d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("GLAD failed to initialize")
        TL_PROFILER_POP_WITH(false)
    }

    TLDEBUG("OPENGL %s", glGetString(GL_VERSION))
    TL_PROFILER_POP_WITH(true)
}

b8 tl_graphics_terminate(void) {
    TL_PROFILER_PUSH
    glfwMakeContextCurrent(NULL);
    TL_PROFILER_POP_WITH(true)
}