#ifndef __TELEIOS_PLATFORM_GLFW__
#define __TELEIOS_PLATFORM_GLFW__

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

static void tl_window_callback_window_closed(GLFWwindow* window) {
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
}

static void tl_window_callback_window_pos(GLFWwindow* _, const int xPos, const int yPos) {
    TLEvent event = { 0 };
    event.i32[0] = xPos;
    event.i32[1] = yPos;

    tl_event_submit(TL_EVENT_WINDOW_MOVED, &event);
}

static void tl_window_callback_window_size(GLFWwindow* window, const int width, const int height) {
    TLEvent event = { 0 };
    event.i32[0] = width;
    event.i32[1] = height;

    tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);
}

static void tl_window_callback_window_focus(GLFWwindow* window, const i32 focused) {
    tl_event_submit(focused ? TL_EVENT_WINDOW_FOCUS_GAINED : TL_EVENT_WINDOW_FOCUS_LOST, NULL);
}

static void tl_window_callback_window_minimized(GLFWwindow* window, const i32 minimized) {
    tl_event_submit(minimized ? TL_EVENT_WINDOW_MINIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_window_maximize(GLFWwindow* window, const i32 maximized) {
    tl_event_submit(maximized ? TL_EVENT_WINDOW_MAXIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_input_keyboard(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
    if (action == GLFW_REPEAT) return;

    i32 type;
    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_KEY_PRESSED;
    } else {
        type = TL_EVENT_INPUT_KEY_RELEASED;
    }

    TLEvent event = { 0 };
    event.u32[0] = key;
    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_position(GLFWwindow* window, const double xpos, const double ypos) {
    TLEvent event = { 0 };
    event.u32[0] = (u32) xpos;
    event.u32[1] = (u32) ypos;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_MOVED, &event);
}

static void tl_window_callback_input_cursor_button(GLFWwindow* window, const int button, const int action, const int mods) {
    i32 type;

    if (action == GLFW_PRESS)   { type = TL_EVENT_INPUT_CURSOR_PRESSED; }
    else                        { type = TL_EVENT_INPUT_CURSOR_RELEASED; }

    TLEvent event = { 0 };
    event.u32[0] = button;

    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_scroll(GLFWwindow* window, const double xoffset, const double yoffset) {
    TLEvent event = { 0 };
    event.u8[0] =  xoffset > 0 ? 1 : xoffset < 0 ? -1 : 0;
    event.u8[1] = yoffset > 0 ? 1 : yoffset < 0 ? -1 : 0;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_SCROLLED, &event);
}

static void tl_window_callback_input_cursor_entered(GLFWwindow* window, const int entered) {
    tl_event_submit(entered ? TL_EVENT_INPUT_CURSOR_ENTERED : TL_EVENT_INPUT_CURSOR_EXITED, NULL);
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
    // --------------------------------------------------------------------------------------
    // Create the platform surface
    // --------------------------------------------------------------------------------------
    m_window_size.x = 1024;
    m_window_size.y = 768;
    m_window = glfwCreateWindow(m_window_size.x, m_window_size.y, "FOSGE", NULL, NULL);
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

    // --------------------------------------------------------------------------------------
    // Window callbacks
    // --------------------------------------------------------------------------------------
    glfwSetWindowCloseCallback      (m_window, tl_window_callback_window_closed);
    glfwSetWindowPosCallback        (m_window, tl_window_callback_window_pos);
    glfwSetWindowSizeCallback       (m_window, tl_window_callback_window_size);
    glfwSetWindowFocusCallback      (m_window, tl_window_callback_window_focus);
    glfwSetWindowIconifyCallback    (m_window, tl_window_callback_window_minimized);
    glfwSetWindowMaximizeCallback   (m_window, tl_window_callback_window_maximize);
    // --------------------------------------------------------------------------------------
    // Input callbacks
    // --------------------------------------------------------------------------------------
    glfwSetKeyCallback              (m_window, tl_window_callback_input_keyboard);
    glfwSetMouseButtonCallback      (m_window, tl_window_callback_input_cursor_button);
    glfwSetCursorPosCallback        (m_window, tl_window_callback_input_cursor_position);
    glfwSetScrollCallback           (m_window, tl_window_callback_input_cursor_scroll);
    glfwSetCursorEnterCallback      (m_window, tl_window_callback_input_cursor_entered);
    
    TL_PROFILER_POP_WITH(true)
}

static void tl_window_terminate(void) {
    TL_PROFILER_PUSH
    glfwDestroyWindow(m_window);
    TL_PROFILER_POP
}
#endif