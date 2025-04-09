#include "teleios/core.h"
#include "teleios/globals.h"

b8 tl_platform_initialize(void) {
    K_FRAME_PUSH

    KDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)

    KTRACE("Initializing GLFW");
    if (!glfwInit()) {
        KERROR("Failed to initialize GLFW")
        K_FRAME_POP_WITH(false)
    }

    KDEBUG("Platform initialized in %llu micros", K_RUNTIME_PROFILER_ELAPSED);
    K_FRAME_POP_WITH(true)
}

b8 tl_platform_terminate(void) {
    K_FRAME_PUSH
    glfwTerminate();
    K_FRAME_POP_WITH(true)
}