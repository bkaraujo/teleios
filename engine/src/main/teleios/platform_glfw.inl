#include "teleios/teleios.h"
#include <GLFW/glfw3.h>

static GLFWwindow* m_window;
static ivec2s m_window_size;
static ivec2s m_window_position;

void* tl_window_handler() {
    return m_window;
}

ivec2s tl_window_size() {
    return m_window_size;
}

ivec2s tl_window_position() {
    return m_window_position;
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

    m_window_size.x = 1024;
    m_window_size.y = 768;
    m_window = glfwCreateWindow(
        m_window_size.x,
        m_window_size.y,
        "FOSGE",
        NULL, NULL
    );

    if (TL_UNLIKELY(m_window == NULL)) {
        TLERROR("GLFW failed to create window")
        TL_PROFILER_POP_WITH(false)
    }

    m_window_position.x = 0;
    m_window_position.y = 0;
    
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (TL_LIKELY(mode)) {
        m_window_position.x = (mode->width - m_window_size.x) / 2;
        m_window_position.y = (mode->height - m_window_size.y) / 2;
    }

    glfwSetWindowPos(m_window, m_window_position.x, m_window_position.y);
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(0); 

    TL_PROFILER_POP_WITH(true)
}