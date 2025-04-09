#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"
// #####################################################################################################################
//
//                                             WINDOWING
//
// #####################################################################################################################
static void tl_window_callback_window_closed(GLFWwindow* window) {
    tl_event_submit(TL_EVENT_WINDOW_CLOSED, NULL);
}

static void tl_window_callback_window_pos(GLFWwindow* _, const int xPos, const int yPos) {
    global->platform.window.position.x = xPos;
    global->platform.window.position.y = yPos;

    TLEvent event = { 0 };
    event.i32[0] = global->platform.window.position.x;
    event.i32[1] = global->platform.window.position.y;

    tl_event_submit(TL_EVENT_WINDOW_MOVED, &event);
}

static void tl_window_callback_window_size(GLFWwindow* window, const int width, const int height) {
    global->platform.window.size.x = width;
    global->platform.window.size.y = height;

    TLEvent event = { 0 };
    event.i32[0] = global->platform.window.size.x;
    event.i32[1] = global->platform.window.size.y;

    tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);
}

static void tl_window_callback_window_focus(GLFWwindow* window, const i32 focused) {
    global->platform.window.focused = focused;

    tl_event_submit(focused ? TL_EVENT_WINDOW_FOCUS_GAINED : TL_EVENT_WINDOW_FOCUS_LOST, NULL);
}

static void tl_window_callback_window_minimized(GLFWwindow* window, const i32 minimized) {
    global->platform.window.maximized = false;
    global->platform.window.minimized = minimized;

    tl_event_submit(minimized ? TL_EVENT_WINDOW_MINIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_window_maximize(GLFWwindow* window, const i32 maximized) {
    global->platform.window.maximized = maximized;
    global->platform.window.minimized = false;

    tl_event_submit(maximized ? TL_EVENT_WINDOW_MAXIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_input_keyboard(GLFWwindow* window, const int key, const int scancode, const int action, const int mods) {
    if (action == GLFW_REPEAT) return;

    i32 type;
    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_KEY_PRESSED;
        global->platform.input.keyboard.key[key] = true;
    } else {
        type = TL_EVENT_INPUT_KEY_RELEASED;
        global->platform.input.keyboard.key[key] = false;
    }

    TLEvent event = { 0 };
    event.u32[0] = key;
    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_position(GLFWwindow* window, const double xpos, const double ypos) {
    global->platform.input.cursor.position_x = (u32) xpos;
    global->platform.input.cursor.position_y = (u32) ypos;

    TLEvent event = { 0 };
    event.u32[0] = global->platform.input.cursor.position_x;
    event.u32[1] = global->platform.input.cursor.position_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_MOVED, &event);
}

static void tl_window_callback_input_cursor_button(GLFWwindow* window, const int button, const int action, const int mods) {
    i32 type;

    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_CURSOR_PRESSED;
        global->platform.input.cursor.button[button] = true;
    } else {
        type = TL_EVENT_INPUT_CURSOR_RELEASED;
        global->platform.input.cursor.button[button] = false;
    }

    global->platform.input.cursor.button[button] = action == GLFW_PRESS;

    TLEvent event = { 0 };
    event.u32[0] = button;

    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_scroll(GLFWwindow* window, const double xoffset, const double yoffset) {
    global->platform.input.cursor.scroll_x = xoffset > 0 ? 1 : xoffset < 0 ? -1 : 0;
    global->platform.input.cursor.scroll_y = yoffset > 0 ? 1 : yoffset < 0 ? -1 : 0;

    TLEvent event = { 0 };
    event.u8[0] = global->platform.input.cursor.scroll_x;
    event.u8[1] = global->platform.input.cursor.scroll_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_SCROLLED, &event);
}

static void tl_window_callback_input_cursor_entered(GLFWwindow* window, const int entered) {
    global->platform.input.cursor.hoover = entered == GLFW_TRUE;
    tl_event_submit(entered ? TL_EVENT_INPUT_CURSOR_ENTERED : TL_EVENT_INPUT_CURSOR_EXITED, NULL);
}

static b8 tl_window_create(void) {
    K_FRAME_PUSH

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
    global->platform.window.handle = glfwCreateWindow(
        global->platform.window.size.x,
        global->platform.window.size.y,
        tl_string(global->platform.window.title),
        NULL, NULL
    );

    if (global->platform.window.handle == NULL) {
        KERROR("Failed to create GLFW window");
        K_FRAME_POP_WITH(false)
    }
    // --------------------------------------------------------------------------------------
    // Cache state
    // --------------------------------------------------------------------------------------
    global->platform.window.visible = false;
    global->platform.window.focused = glfwGetWindowAttrib(global->platform.window.handle, GLFW_FOCUSED) == GLFW_TRUE;
    global->platform.window.maximized = glfwGetWindowAttrib(global->platform.window.handle, GLFW_MAXIMIZED) == GLFW_TRUE;
    global->platform.window.minimized = glfwGetWindowAttrib(global->platform.window.handle, GLFW_ICONIFIED) == GLFW_TRUE;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (mode != NULL) {
        global->platform.window.position.x = (mode->width - global->platform.window.size.x) / 2;
        global->platform.window.position.y = (mode->height - global->platform.window.size.y) / 2;
    }

    glfwSetWindowPos(global->platform.window.handle, global->platform.window.position.x, global->platform.window.position.y);
    // --------------------------------------------------------------------------------------
    // Window callbacks
    // --------------------------------------------------------------------------------------
    glfwSetWindowCloseCallback      (global->platform.window.handle, tl_window_callback_window_closed);
    glfwSetWindowPosCallback        (global->platform.window.handle, tl_window_callback_window_pos);
    glfwSetWindowSizeCallback       (global->platform.window.handle, tl_window_callback_window_size);
    glfwSetWindowFocusCallback      (global->platform.window.handle, tl_window_callback_window_focus);
    glfwSetWindowIconifyCallback    (global->platform.window.handle, tl_window_callback_window_minimized);
    glfwSetWindowMaximizeCallback   (global->platform.window.handle, tl_window_callback_window_maximize);
    // --------------------------------------------------------------------------------------
    // Input callbacks
    // --------------------------------------------------------------------------------------
    glfwSetKeyCallback              (global->platform.window.handle, tl_window_callback_input_keyboard);
    glfwSetMouseButtonCallback      (global->platform.window.handle, tl_window_callback_input_cursor_button);
    glfwSetCursorPosCallback        (global->platform.window.handle, tl_window_callback_input_cursor_position);
    glfwSetScrollCallback           (global->platform.window.handle, tl_window_callback_input_cursor_scroll);
    glfwSetCursorEnterCallback      (global->platform.window.handle, tl_window_callback_input_cursor_entered);

    K_FRAME_POP_WITH(true)
}
// #####################################################################################################################
//
//                                                     LIFECYCLE
//
// #####################################################################################################################
b8 tl_runtime_initialize(void) {
    K_FRAME_PUSH

    if (global->properties == NULL || tl_map_length(global->properties) == 0) {
        KERROR("Failed to read runtime properties")
        K_FRAME_POP_WITH(false)
    }

    TLMemoryArena *scrape = tl_memory_arena_create(K_MEBI_BYTES(1));
    TLIterator *it = tl_map_keys(scrape, global->properties);
    for (TLString* key = tl_iterator_next(it); key != NULL; key = tl_iterator_next(it)) {
        if (tl_string_start_with(key, "application.scenes.")) continue;
        const char *value = tl_string(tl_map_get(global->properties, tl_string(key)));

        if (tl_string_equals(key, "application.version")) {
            global->application.version = tl_string_wrap(global->arena, value);
            KTRACE("global->application.version = %s", tl_string(global->application.version))
            continue;
        }

        if ( ! tl_string_start_with(key, "engine.")) continue;

        if (tl_string_equals(key, "engine.logging.level")) {
            if (tl_char_equals(value, "verbose")) { k_logger_loglevel(K_LOG_LEVEL_VERBOSE); continue; }
            if (tl_char_equals(value,   "trace")) { k_logger_loglevel(  K_LOG_LEVEL_TRACE); continue; }
            if (tl_char_equals(value,   "debug")) { k_logger_loglevel(  K_LOG_LEVEL_DEBUG); continue; }
            if (tl_char_equals(value,    "info")) { k_logger_loglevel(   K_LOG_LEVEL_INFO); continue; }
            if (tl_char_equals(value,    "warn")) { k_logger_loglevel(   K_LOG_LEVEL_WARN); continue; }
            if (tl_char_equals(value,   "error")) { k_logger_loglevel(  K_LOG_LEVEL_ERROR); continue; }
            if (tl_char_equals(value,   "fatal")) { k_logger_loglevel(  K_LOG_LEVEL_FATAL); continue; }
        }


        if (tl_string_start_with(key, "engine.graphics.")) {
            if (tl_string_equals(key, "engine.graphics.vsync")) {
                global->platform.graphics.vsync = tl_char_equals(value, "true");
                KTRACE("global->platform.graphics.vsync = %d", global->platform.graphics.vsync)
                continue;
            }

            if (tl_string_equals(key, "engine.graphics.wireframe")) {
                global->platform.graphics.wireframe = tl_char_equals(value, "true");
                KTRACE("global->platform.graphics.wireframe = %d", global->platform.graphics.wireframe)
                continue;
            }
        }

        if (tl_string_start_with(key, "engine.simulation.")) {
            if (tl_string_equals(key, "engine.simulation.step")) {
                u8 step = strtol(value, (void*)(value + tl_char_length(value)), 10);
                if (step == 0) {
                    KWARN("Failed to read [%s] assuming 24", tl_string(key));
                    step = 24;
                }

                global->application.simulation.step = 1.0f / (f64) step;
                KTRACE("global->simulation.step = %f", global->application.simulation.step)
                continue;
            }
        }

        if (tl_string_start_with(key, "engine.window.")) {
            if (tl_string_equals(key, "engine.window.title")) {
                global->platform.window.title = tl_string_clone(global->arena, value);
                KTRACE("global->platform.window.title = %s", tl_string(global->platform.window.title))
                continue;
            }

            if (tl_string_equals(key, "engine.window.size")) {
                if (tl_char_equals(value,  "SD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_SD; }
                if (tl_char_equals(value,  "HD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_HD; }
                if (tl_char_equals(value, "FHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_FHD; }
                if (tl_char_equals(value, "QHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_QHD; }
                if (tl_char_equals(value, "UHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_UHD; }

                global->platform.window.size.y = (global->platform.window.size.x * 9) / 16;
                KTRACE("global->platform.window.size = %u x %u", global->platform.window.size.x, global->platform.window.size.y)
                continue;
            }
        }
    }
    tl_memory_arena_destroy(scrape);

    if (!tl_thread_initialize()) {
        KERROR("Failed to initialize threadpool")
        K_FRAME_POP_WITH(false)
    }

    if (!tl_window_create()) {
        KERROR("Failed to create application window");
        K_FRAME_POP_WITH(false)
    }

    if (!tl_script_initialize()) {
        KERROR("Failed to initialize script engine");
        K_FRAME_POP_WITH(false)
    }

    if (!tl_graphics_initialize()) {
        KERROR("Failed to initialize Graphics API")
        K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(true)
}

b8 tl_runtime_terminate(void) {
    K_FRAME_PUSH

    if (!tl_script_terminate()) {
        KERROR("Failed to terminate script engine");
        K_FRAME_POP_WITH(false)
    }

    if (!tl_graphics_terminate()) {
        KERROR("Failed to terminate graphics engine");
        K_FRAME_POP_WITH(false)
    }


    if (!tl_thread_terminate()) {
        KERROR("Failed to terminate threadpool")
        K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(true)
}