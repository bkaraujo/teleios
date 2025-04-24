#include "teleios/teleios.h"
#include "glfw/glfw3.h"

static TLRuntime runtime = { 0 };

b8 tl_runtime_initialize(const char *yaml) {
    K_FRAME_PUSH

    KDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)
    if (!glfwInit()) {
        KERROR("Failed to initialize GLFW")
        K_FRAME_POP_WITH(false)
    }

    runtime.allocator = k_memory_allocator_create(K_MEMORY_ALLOCATOR_LINEAR, K_MEBI_BYTES(10));
    runtime.yaml = k_map_create(runtime.allocator);
    tl_yaml_walk(yaml, runtime.yaml);

    KDEBUG("Platform initialized in %llu micros", K_RUNTIME_PROFILER_ELAPSED);
    K_FRAME_POP_WITH(true)
}

TLRuntime* tl_runtime_get() {
    return &runtime;
}

b8 tl_runtime_terminate(void) {
    K_FRAME_PUSH
    glfwTerminate();
    K_FRAME_POP_WITH(true)
}