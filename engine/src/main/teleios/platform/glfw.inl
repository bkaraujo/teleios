#ifndef __TELEIOS_PLATFORM_GLFW__
#define __TELEIOS_PLATFORM_GLFW__

#include "teleios/teleios.h"
#include <GLFW/glfw3.h>

static GLFWwindow* m_window;
static ivec2s m_window_size;
static ivec2s m_window_position;
static TLString* m_window_title;

void* tl_window_handler() {
    return m_window;
}

ivec2s tl_window_size() {
    return m_window_size;
}

ivec2s tl_window_position() {
    return m_window_position;
}

TLString* tl_window_title() {
    return m_window_title;
}

static void tl_window_callback_window_closed(GLFWwindow* window);
static void tl_window_callback_window_pos(GLFWwindow* window, i32 xPos, i32 yPos);
static void tl_window_callback_window_size(GLFWwindow* window, i32 width, i32 height);
static void tl_window_callback_window_focus(GLFWwindow* window, i32 focused);
static void tl_window_callback_window_minimized(GLFWwindow* window, i32 minimized);
static void tl_window_callback_window_maximize(GLFWwindow* window, i32 maximized);
static void tl_window_callback_input_keyboard(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
static void tl_window_callback_input_cursor_position(GLFWwindow* window, f64 xpos, f64 ypos);
static void tl_window_callback_input_cursor_button(GLFWwindow* window, i32 button, i32 action, i32 mods);
static void tl_window_callback_input_cursor_scroll(GLFWwindow* window, f64 xoffset, f64 yoffset);
static void tl_window_callback_input_cursor_entered(GLFWwindow* window, i32 entered);

GLFWwindow* tl_glfw_create_window(const i32 width, const i32 height, const char* title) {
    // OpenGL versions to try, in descending order (major.minor)
    const i32 gl_versions[][2] = {
        {4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
        {3, 3}, {3, 2}, {3, 1}, {3, 0}
    };
    const i32 num_versions = sizeof(gl_versions) / sizeof(gl_versions[0]);

    GLFWwindow* window = NULL;

    for (i32 i = 0; i < num_versions; i++) {
        const i32 major = gl_versions[i][0];
        const i32 minor = gl_versions[i][1];

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (window != NULL) { return window; }
    }

    return NULL;
}

static b8 tl_window_create(void) {
    TL_PROFILER_PUSH

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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

    const TLDisplayResolution resolution = tl_config_get_display_resolution("teleios.window.size");
    m_window_size.x = (resolution * 16) / 9;
    m_window_size.y = resolution;
    m_window_title = tl_config_get("teleios.window.title");

    m_window = tl_glfw_create_window(m_window_size.x, m_window_size.y, tl_string_cstr(m_window_title));
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

static void tl_window_callback_window_closed(GLFWwindow* window) {
    (void) window;  // Unused parameter
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
}

static void tl_window_callback_window_pos(GLFWwindow* window, const i32 xPos, const i32 yPos) {
    (void) window;  // Unused parameter
    TLEvent event = { 0 };
    event.i32[0] = xPos;
    event.i32[1] = yPos;

    tl_event_submit(TL_EVENT_WINDOW_MOVED, &event);
}

static void tl_window_callback_window_size(GLFWwindow* window, const i32 width, const i32 height) {
    (void) window;  // Unused parameter
    TLEvent event = { 0 };
    event.i32[0] = width;
    event.i32[1] = height;

    tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);
}

static void tl_window_callback_window_focus(GLFWwindow* window, const i32 focused) {
    (void) window;  // Unused parameter
    tl_event_submit(focused ? TL_EVENT_WINDOW_FOCUS_GAINED : TL_EVENT_WINDOW_FOCUS_LOST, NULL);
}

static void tl_window_callback_window_minimized(GLFWwindow* window, const i32 minimized) {
    (void) window;  // Unused parameter
    tl_event_submit(minimized ? TL_EVENT_WINDOW_MINIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_window_maximize(GLFWwindow* window, const i32 maximized) {
    (void) window;  // Unused parameter
    tl_event_submit(maximized ? TL_EVENT_WINDOW_MAXIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_input_keyboard(GLFWwindow* window, const i32 key, const i32 scancode, const i32 action, const i32 mods) {
    (void) window;  // Unused parameter
    (void) scancode;  // Unused parameter
    (void) mods;  // Unused parameter
    if (action == GLFW_REPEAT) return;

    u16 type;
    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_KEY_PRESSED;
    } else {
        type = TL_EVENT_INPUT_KEY_RELEASED;
    }

    TLEvent event = { 0 };
    event.i32[0] = (i32)key;
    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_position(GLFWwindow* window, const f64 xpos, const f64 ypos) {
    (void) window;  // Unused parameter
    TLEvent event = { 0 };
    event.f32[0] = (f32) xpos;
    event.f32[1] = (f32) ypos;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_MOVED, &event);
}

static void tl_window_callback_input_cursor_button(GLFWwindow* window, const i32 button, const i32 action, const i32 mods) {
    (void) window;  // Unused parameter
    (void) mods;  // Unused parameter

    u16 type;
    if (action == GLFW_PRESS)   { type = TL_EVENT_INPUT_CURSOR_PRESSED; }
    else                        { type = TL_EVENT_INPUT_CURSOR_RELEASED; }

    TLEvent event = { 0 };
    event.i32[0] = button;

    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_scroll(GLFWwindow* window, const f64 xoffset, const f64 yoffset) {
    (void) window;  // Unused parameter
    TLEvent event = { 0 };
    event.i8[0] =  xoffset > 0 ? 1 : (i8)(xoffset < 0 ? -1 : 0);
    event.i8[1] = yoffset > 0 ? 1 : (i8)(yoffset < 0 ? -1 : 0);

    tl_event_submit(TL_EVENT_INPUT_CURSOR_SCROLLED, &event);
}

static void tl_window_callback_input_cursor_entered(GLFWwindow* window, const i32 entered) {
    (void) window;  // Unused parameter
    tl_event_submit(entered ? TL_EVENT_INPUT_CURSOR_ENTERED : TL_EVENT_INPUT_CURSOR_EXITED, NULL);
}

static void tl_window_terminate(void) {
    TL_PROFILER_PUSH
    glfwDestroyWindow(m_window);
    TL_PROFILER_POP
}
#endif