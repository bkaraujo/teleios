#include "teleios/core.h"
#include "teleios/globals.h"

b8 tl_platform_initialize(void) {
    BKS_TRACE_PUSH

    BKSDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)

    BKSTRACE("Initializing GLFW");
    if (!glfwInit()) {
        BKSERROR("Failed to initialize GLFW")
        BKS_TRACE_POPV(false)
    }

    BKSDEBUG("Platform initialized in %llu micros", BKS_PROFILER_MICROS);
    BKS_TRACE_POPV(true)
}

b8 tl_platform_terminate(void) {
    BKS_TRACE_PUSH

    glfwTerminate();
    BKS_TRACE_POPV(true)
}