#include "teleios/application.h"
#include "teleios/logger.h"
#include "teleios/container.h"
#include "teleios/platform.h"
#include "teleios/memory.h"
#include "GLFW/glfw3.h"

static GLFWwindow* window;
static TLMemoryArena* arena;

b8 tl_application_initialize(void) {
    TLVERBOSE(">> tl_application_initialize(void)")

    arena = tl_memory_arena_create(TLMEBIBYTES(10));

    TLVERBOSE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        return FALSE;
    }
    
    TLVERBOSE("Creating window");
    window = glfwCreateWindow(640, 480, "Teleios APP", NULL, NULL);
    if (window == NULL) {
        TLERROR("Failed to create GLFW window");
        
    }

    TLMemoryArena* list_arena = tl_memory_arena_create(TLMEBIBYTES(10));
    TLList* list = tl_list_create(list_arena);

    TLVERBOSE("<< tl_application_initialize(void)")
    return TRUE;
}

b8 tl_application_run(void) {
    TLVERBOSE(">> tl_application_run(void)")

    glfwShowWindow(window);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwHideWindow(window);

    TLVERBOSE("<< tl_application_run(void)")
    return TRUE;
}

b8 tl_application_terminate(void) {
    TLVERBOSE(">> tl_application_terminate(void)")
    tl_memory_arena_destroy(arena);
    TLVERBOSE("Terminating GLFW");
    glfwTerminate();
    TLVERBOSE("<< tl_application_terminate(void)")
    return TRUE;
}