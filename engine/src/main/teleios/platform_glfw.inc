#include "teleios/teleios.h"
#include <GLFW/glfw3.h>

void* m_window;

void* tl_window_handler() {
    return m_window;
}

static b8 tl_window_create(void) {
    TL_PROFILER_PUSH
        
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_true);
#endif

    // --------------------------------------------------------------------------------------
    // Disable window framebuffer bits we don't need, because we render into offscreen FBO and blit to window.
    // --------------------------------------------------------------------------------------
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);

    u32 size_x = 1024, size_y = 768;
    m_window = glfwCreateWindow(
        size_x,
        size_y,
        "FOSGE",
        NULL, NULL
    );

    if (TL_UNLIKELY(m_window == NULL)) {
        TLERROR("GLFW failed to create window")
        TL_PROFILER_POP_WITH(false)
    }

    u32 position_x = 0, position_y = 0;
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (TL_LIKELY(mode)) {
        position_x = (mode->width - size_x) / 2;
        position_y = (mode->height - size_y) / 2;
    }

    glfwSetWindowPos(m_window, position_x, position_y);
       TL_PROFILER_POP_WITH(true)
}