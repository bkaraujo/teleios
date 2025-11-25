#include "teleios/teleios.h"
#include "teleios/graphics.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cglm/version.h>

static void* tl_graphics_thread(void*);
static TLThread* m_thread;

b8 tl_graphics_initialize(void) {
    TL_PROFILER_PUSH

    m_thread = tl_thread_create(global->allocator, tl_graphics_thread, NULL);
    if (m_thread == NULL) TLFATAL("Failed to create Graphics Thread")

    TL_PROFILER_POP_WITH(true)
}

static void* tl_graphics_thread(void* _) {
    (void) _;

    // #########################################
    // Initialize OpenGL context
    // #########################################
    glfwMakeContextCurrent(tl_window_handler());
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLFATAL("GLAD failed to initialize on graphics thread")
    }

    TLDEBUG("OpenGL %s", glGetString(GL_VERSION))
    TLDEBUG("CGLM %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)
    // #########################################
    // Process global application.yaml options
    // #########################################
    const b8 vsync = tl_config_get_b8("teleios.graphics.vsync");
    glfwSwapInterval(vsync ? 1 : 0);

    const b8 wireframe = tl_config_get_b8("teleios.graphics.wireframe");
    if (wireframe)  { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    else            { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

    glClearColor(0.126f, 0.48f, 1.0f, 1.0f);

    TLDEBUG("Entering Main Loop")
    for ( ; global->running ; ) {

    }

    glfwMakeContextCurrent(NULL);
    TLDEBUG("Finalizando")

    return NULL;
}

b8 tl_graphics_terminate(void) {
    TL_PROFILER_PUSH

    if (m_thread) {
        tl_thread_join(m_thread, NULL);
        m_thread = NULL;
    }

    TL_PROFILER_POP_WITH(true)
}