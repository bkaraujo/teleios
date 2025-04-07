#include "teleios/core.h"
#include "teleios/globals.h"

BKS_INLINE void* tl_platform_memory_alloc(const u64 size) {
    return __builtin_malloc(size);
}

BKS_INLINE void tl_platform_memory_free(void *block) {
    __builtin_free(block);
}
BKS_INLINE void tl_platform_memory_set(void *block, const i32 value, const u64 size) {
    __builtin_memset(block, value, size);
}

BKS_INLINE void tl_platform_memory_copy(void *target, const void *source, const u64 size) {
    __builtin_memcpy(target, source, size);
}

b8 tl_platform_initialize(void) {
    BKS_STACK_PUSH

    BKSDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)

    BKSTRACE("Initializing GLFW");
    if (!glfwInit()) {
        BKSERROR("Failed to initialize GLFW")
        BKS_STACK_POPV(false)
    }

    BKSDEBUG("Platform initialized in %llu micros", BKS_PROFILER_MICROS);
    BKS_STACK_POPV(true)
}

b8 tl_platform_terminate(void) {
    BKS_STACK_PUSH

    glfwTerminate();
    BKS_STACK_POPV(true)
}