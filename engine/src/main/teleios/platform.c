#include "teleios/core.h"
#include "teleios/globals.h"

// #####################################################################################################################
//
//                                                     Platform
//
// #####################################################################################################################
TL_INLINE void* tl_platform_memory_alloc(const u64 size) {
    return __builtin_malloc(size);
}

TL_INLINE void tl_platform_memory_free(void *block) {
    __builtin_free(block);
}
TL_INLINE void tl_platform_memory_set(void *block, const i32 value, const u64 size) {
    __builtin_memset(block, value, size);
}

TL_INLINE void tl_platform_memory_copy(void *target, const void *source, const u64 size) {
    __builtin_memcpy(target, source, size);
}

b8 tl_platform_initialize(void) {
    TL_STACK_PUSH

    TLDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)

    TLTRACE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        TL_STACK_POPV(false)
    }

    TLDEBUG("Platform initialized in %llu micros", TL_PROFILER_MICROS);
    TL_STACK_POPV(true)
}

b8 tl_platform_terminate(void) {
    TL_STACK_PUSH

    glfwTerminate();
    TL_STACK_POPV(true)
}