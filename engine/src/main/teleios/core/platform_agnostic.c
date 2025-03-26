#include "teleios/core.h"
#include "teleios/globals.h"

// ########################################################
//                    MEMORY FUNCTIONS
// ########################################################
typedef struct {
    u64 index;
    char *payload;
} TLMemoryPage;

struct TLMemoryArena {
    u64 allocated;
    u64 tagged_count[TL_MEMORY_MAXIMUM];
    u64 tagged_size[TL_MEMORY_MAXIMUM];
    u64 page_size;
    TLMemoryPage page[U8_MAX];
};

static const char* tl_memory_name(const TLMemoryTag tag) {
    TLSTACKPUSHA("%d", tag)
    switch (tag) {
        case TL_MEMORY_BLOCK                : TLSTACKPOPV("TL_MEMORY_BLOCK")
        case TL_MEMORY_SERIALIZER           : TLSTACKPOPV("TL_MEMORY_SERIALIZER")
        case TL_MEMORY_CONTAINER_LIST       : TLSTACKPOPV("TL_MEMORY_CONTAINER_LIST")
        case TL_MEMORY_CONTAINER_STACK      : TLSTACKPOPV("TL_MEMORY_CONTAINER_STACK")
        case TL_MEMORY_CONTAINER_NODE       : TLSTACKPOPV("TL_MEMORY_CONTAINER_NODE")
        case TL_MEMORY_CONTAINER_ITERATOR   : TLSTACKPOPV("TL_MEMORY_CONTAINER_ITERATOR")
        case TL_MEMORY_STRING               : TLSTACKPOPV("TL_MEMORY_STRING")
        case TL_MEMORY_PROFILER             : TLSTACKPOPV("TL_MEMORY_PROFILER")
        case TL_MEMORY_SCENE                : TLSTACKPOPV("TL_MEMORY_SCENE")
        case TL_MEMORY_ULID                 : TLSTACKPOPV("TL_MEMORY_ULID")
        case TL_MEMORY_MAXIMUM              : TLSTACKPOPV("TL_MEMORY_MAXIMUM")
    }

    TLSTACKPOPV("???")
}

TLMemoryArena* tl_memory_arena_create(const u64 size) {
    TLSTACKPUSHA("%d", size)
    // ----------------------------------------------------------
    // Create the memory arena
    // ----------------------------------------------------------
    TLMemoryArena *arena = TLMALLOC(sizeof(TLMemoryArena));
    if (arena == NULL) TLFATAL("Failed to allocate TLMemoryArena");
    TLMEMSET(arena, 0, sizeof(TLMemoryArena));
    arena->page_size = size;
    // ----------------------------------------------------------
    // Keep track of the created arena
    // ----------------------------------------------------------
    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->platform.memory.arenas[i] == NULL) {
            global->platform.memory.arenas[i] = arena;
            TLVERBOSE("TLMemoryArena 0x%p created with page size of %d bytes", arena, arena->page_size)
            TLSTACKPOPV(arena)
        }
    }

    TLFATAL("Failed to allocate TLMemoryArena");
}

TLINLINE static u8 tl_memory_arena_get_index(TLMemoryArena *arena) {
    TLSTACKPUSHA("0x%p", arena)

    if (arena == NULL) {
        TLWARN("TLMemoryArena is NULL")
        TLSTACKPOPV(FALSE)
    }

    for (u8 i = 0 ; i < U8_MAX ; ++i) {
        if (global->platform.memory.arenas[i] == NULL) continue;
        if (global->platform.memory.arenas[i] == arena) {
            TLSTACKPOPV(i)
        }
    }

    TLFATAL("TLMemoryArena 0x%p not found", arena)
}

TLINLINE static void tl_memory_arena_do_destroy(const u8 index) {
    TLSTACKPUSHA("%d", index)
    TLMemoryArena *arena = global->platform.memory.arenas[index];
    for (u32 i = 0 ; i < TLARRSIZE(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload != NULL) {
            TLVERBOSE("TLMemoryArena 0x%p releasing page %d", arena, i)
            TLFREE(arena->page[i].payload);
            arena->page[i].payload = NULL;
        }
    }

    for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
        if (global->platform.memory.arenas[index]->tagged_size[i] != 0) {
            TLVERBOSE("TLMemoryArena 0x%p at %-30s: [%03d] %llu bytes", arena, tl_memory_name(i), arena->tagged_count[i], arena->tagged_size[i]);
        }
    }

    TLFREE(arena);
    global->platform.memory.arenas[index] = NULL;

    TLSTACKPOP
}

void tl_memory_arena_reset(TLMemoryArena *arena) {
    TLSTACKPUSHA("0x%p", arena)
    for (u32 i = 0 ; i < TLARRSIZE(arena->page, TLMemoryPage) ; ++i) {
        if (arena->page[i].payload == NULL) break;

        arena->page[i].index = 0;
        tl_memory_set(arena->page[i].payload, 0, arena->page_size);
    }
    TLSTACKPOP
}

void tl_memory_arena_destroy(TLMemoryArena *arena) {
    TLSTACKPUSHA("0x%p", arena)
    const u8 index = tl_memory_arena_get_index(arena);
    tl_memory_arena_do_destroy(index);
    TLSTACKPOP
}

void *tl_memory_alloc(TLMemoryArena *arena, const u64 size, const TLMemoryTag tag) {
    TLSTACKPUSHA("0x%p, %llu, %d", arena, size, tag)
    // -------------------------------------------------
    // Ensure that the Arena can hold the desired size
    // -------------------------------------------------
    if (size == 0) {
        TLFATAL("TLMemoryArena 0x%p allocation size must be greater then 0", arena)
        TLSTACKPOPV(NULL)
    }

    if (size > arena->page_size) {
        TLFATAL("TLMemoryArena with page size of %d bytes. It cannot allocate %d bytes", arena, arena->page_size, size)
        TLSTACKPOPV(NULL)
    }
    // -------------------------------------------------
    // Find a suitable TLMemoryPage within the arena
    // -------------------------------------------------
    u8 found = U8_MAX;
    for (u8 i = 0; i < U8_MAX ; ++i) {

        // Initialize a new TLMemoryPage
        if (arena->page[i].payload == NULL) {
            TLVERBOSE("TLMemoryArena 0x%p initializing page %d", arena, i)
            arena->page[i].payload = TLMALLOC(arena->page_size);
            TLMEMSET(arena->page[i].payload, 0, arena->page_size);

            found = i;
            break;
        }

        // check if the page support the desired size
        if (arena->page[i].index + size <= arena->page_size) {
            found = i;
            break;
        }
    }
    
    if (found == U8_MAX) {
        TLWARN("TLMemoryArena 0x%p no suitable TLMemoryPage", arena)
        TLSTACKPOPV(NULL)
    }
    // -------------------------------------------------
    // Adjust the TLMemoryArena internal state
    // -------------------------------------------------
    arena->allocated += size;
    arena->tagged_count[tag] += 1;
    arena->tagged_size[tag] += size;
    // -------------------------------------------------
    // Adjust the TLMemoryPage internal state
    // -------------------------------------------------
    void* address = arena->page[found].payload + arena->page[found].index;
    TLVERBOSE("TLMemoryArena 0x%p page %d [remaning %llu] :: allocating %llu ", arena, found, arena->page_size - arena->page[found].index, size)
    arena->page[found].index += size;
    // -------------------------------------------------
    // Hand out the memory pointer
    // -------------------------------------------------
    TLSTACKPOPV(address)
}

void tl_memory_set(void *block, const i32 value, const u64 size) {
    TLSTACKPUSHA("0x%p, %d, %llu", block, value, size)
    TLMEMSET(block, value, size);
    TLSTACKPOP
}

void tl_memory_copy(void *target, void *source, const u64 size) {
    TLSTACKPUSHA("0x%p, 0x%p, %llu", target, source, size)
    TLMEMCPY(target, source, size);
    TLSTACKPOP
}
// ########################################################
//                  WINDOWING FUNCTIONS
// ########################################################
static void tl_window_callback_window_closed(GLFWwindow* _) {
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

static void tl_window_callback_window_size(GLFWwindow* _, const int width, const int height) {
    global->platform.window.size.x = width;
    global->platform.window.size.y = height;

    TLEvent event = { 0 };
    event.i32[0] = global->platform.window.size.x;
    event.i32[1] = global->platform.window.size.y;

    tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);
}

static void tl_window_callback_window_focus(GLFWwindow* _, const i32 focused) {
    global->platform.window.focused = focused;

    tl_event_submit(focused ? TL_EVENT_WINDOW_FOCUS_GAINED : TL_EVENT_WINDOW_FOCUS_LOST, NULL);
}

static void tl_window_callback_window_minimized(GLFWwindow* _, const i32 minimized) {
    global->platform.window.maximized = FALSE;
    global->platform.window.minimized = minimized;

    tl_event_submit(minimized ? TL_EVENT_WINDOW_MINIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}

static void tl_window_callback_window_maximize(GLFWwindow* _, const i32 maximized) {
    global->platform.window.maximized = maximized;
    global->platform.window.minimized = FALSE;

    tl_event_submit(maximized ? TL_EVENT_WINDOW_MAXIMIZED : TL_EVENT_WINDOW_RESTORED, NULL);
}
// ########################################################
//                  INPUT FUNCTIONS
// ########################################################
static void tl_window_callback_input_keyboard(GLFWwindow* _, const int key, const int scancode, const int action, const int mods) {
    i32 type;

    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_KEY_PRESSED;
        global->platform.input.keyboard.key[key] = TRUE;
    } else {
        type = TL_EVENT_INPUT_KEY_RELEASED;
        global->platform.input.keyboard.key[key] = FALSE;
    }

    TLEvent event = { 0 };
    event.u32[0] = key;
    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_position(GLFWwindow* _, const double xpos, const double ypos) {
    global->platform.input.cursor.position_x = (u32) xpos;
    global->platform.input.cursor.position_y = (u32) ypos;

    TLEvent event = { 0 };
    event.u32[0] = global->platform.input.cursor.position_x;
    event.u32[1] = global->platform.input.cursor.position_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_MOVED, &event);
}

static void tl_window_callback_input_cursor_button(GLFWwindow* _, const int button, const int action, const int mods) {
    i32 type;

    if (action == GLFW_PRESS) {
        type = TL_EVENT_INPUT_CURSOR_PRESSED;
        global->platform.input.cursor.button[button] = TRUE;
    } else {
        type = TL_EVENT_INPUT_CURSOR_RELEASED;
        global->platform.input.cursor.button[button] = FALSE;
    }

    global->platform.input.cursor.button[button] = action == GLFW_PRESS;

    TLEvent event = { 0 };
    event.u32[0] = button;

    tl_event_submit(type, &event);
}

static void tl_window_callback_input_cursor_scroll(GLFWwindow* _, const double xoffset, const double yoffset) {
    global->platform.input.cursor.scroll_x = xoffset > 0 ? 1 : xoffset < 0 ? -1 : 0;
    global->platform.input.cursor.scroll_y = yoffset > 0 ? 1 : yoffset < 0 ? -1 : 0;

    TLEvent event = { 0 };
    event.u8[0] = global->platform.input.cursor.scroll_x;
    event.u8[1] = global->platform.input.cursor.scroll_y;

    tl_event_submit(TL_EVENT_INPUT_CURSOR_SCROLLED, &event);
}

static void tl_window_callback_input_cursor_entered(GLFWwindow* _, const int entered) {
    global->platform.input.cursor.hoover = entered == GLFW_TRUE;
    tl_event_submit(entered ? TL_EVENT_INPUT_CURSOR_ENTERED : TL_EVENT_INPUT_CURSOR_EXITED, NULL);
}
// ########################################################
//                  LIFECYCLE FUNCTIONS
// ########################################################
#include "teleios/core/platform.h"

static void tl_serializer_read(const char *prefix, const char *element, const char *value, u64 length);

b8 tl_platform_initialize(void) {
    TLSTACKPUSH
    global->application.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.scene.arena = tl_memory_arena_create(TLMEBIBYTES(10));
    global->application.frame.arena = tl_memory_arena_create(TLMEBIBYTES(10));

    tl_serializer_walk(tl_serializer_read);

    TLDEBUG("GLFW_VERSION %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION)
    // --------------------------------------------------------------------------------------
    // Initialize GLFW
    // --------------------------------------------------------------------------------------
    TLTRACE("Initializing GLFW");
    if (!glfwInit()) {
        TLERROR("Failed to initialize GLFW")
        TLSTACKPOPV(FALSE)
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
#ifdef TLPLATFORM_APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Disable window framebuffer bits we don't need, because we render into offscreen FBO and blit to window.

    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_ALPHA_BITS, 0);
    // --------------------------------------------------------------------------------------
    // Create the platform surface
    // --------------------------------------------------------------------------------------
    TLDEBUG(
        "Window (%u x %u) :: %s",
        global->platform.window.size.x,
        global->platform.window.size.y,
        tl_string(global->platform.window.title)
    )

    global->platform.window.handle = glfwCreateWindow(
        global->platform.window.size.x,
        global->platform.window.size.y,
        tl_string(global->platform.window.title),
        NULL, NULL
    );
    
    if (global->platform.window.handle == NULL) {
        TLERROR("Failed to create GLFW window");
        TLSTACKPOPV(FALSE)
    }
    // --------------------------------------------------------------------------------------
    // Cache state
    // --------------------------------------------------------------------------------------
    global->platform.window.visible = FALSE;
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
    // Register callbacks
    // --------------------------------------------------------------------------------------
    glfwSetWindowCloseCallback(global->platform.window.handle, tl_window_callback_window_closed);
    glfwSetWindowPosCallback(global->platform.window.handle, tl_window_callback_window_pos);
    glfwSetWindowSizeCallback(global->platform.window.handle, tl_window_callback_window_size);
    glfwSetWindowFocusCallback(global->platform.window.handle, tl_window_callback_window_focus);
    glfwSetWindowIconifyCallback(global->platform.window.handle, tl_window_callback_window_minimized);
    glfwSetWindowMaximizeCallback(global->platform.window.handle, tl_window_callback_window_maximize);

    glfwSetKeyCallback(global->platform.window.handle, tl_window_callback_input_keyboard);
    glfwSetMouseButtonCallback(global->platform.window.handle, tl_window_callback_input_cursor_button);
    glfwSetCursorPosCallback(global->platform.window.handle, tl_window_callback_input_cursor_position);
    glfwSetScrollCallback(global->platform.window.handle, tl_window_callback_input_cursor_scroll);
    glfwSetCursorEnterCallback(global->platform.window.handle, tl_window_callback_input_cursor_entered);
    // --------------------------------------------------------------------------------------
    // Initialize graphics
    // --------------------------------------------------------------------------------------
    glfwMakeContextCurrent(global->platform.window.handle);

    TLDEBUG("CGLM_VERSION %d.%d.%d", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH)

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        TLERROR("Failed to initialize GLAD")
        TLSTACKPOPV(FALSE)
    }

    TLDEBUG("GL_VERSION %s", glGetString(GL_VERSION))

    if (global->platform.graphics.vsync) {
        TLDEBUG("vsync: on")
        glfwSwapInterval(1);
    } else {
        TLDEBUG("vsync: off")
        glfwSwapInterval(0);
    }

    if (global->platform.graphics.wireframe) {
        TLDEBUG("wireframe: on")
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        TLDEBUG("wireframe: off")
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    TLDEBUG("Platform initialized in %llu micros", TLPROFILER_MICROS);
    TLSTACKPOPV(TRUE)
}

static void tl_serializer_read(const char *prefix, const char *element, const char *value, const u64 length) {
    TLSTACKPUSHA("%s, %s, %s", prefix, element, value)

    if (tl_char_start_with(prefix, "application.")) {
        if (tl_char_start_with(prefix, "application.scenes.")) TLSTACKPOP

        if (tl_char_equals(prefix, "application.")) {
            if (tl_char_equals(element, "version")) {
                global->application.version = tl_string_clone(global->platform.arena, value);
                TLTRACE("global->application.version = %s", value)
                TLSTACKPOP
            }
        }

        TLSTACKPOP
    }

    if (tl_char_start_with(prefix, "engine.")) {
        if (tl_char_equals(prefix, "engine.logging.") && tl_char_equals(element, "level")) {
            if (tl_char_equals(value, "verbose")) { tl_logger_loglevel(TL_LOG_LEVEL_VERBOSE); TLSTACKPOP }
            if (tl_char_equals(value,   "trace")) { tl_logger_loglevel(  TL_LOG_LEVEL_TRACE); TLSTACKPOP }
            if (tl_char_equals(value,   "debug")) { tl_logger_loglevel(  TL_LOG_LEVEL_DEBUG); TLSTACKPOP }
            if (tl_char_equals(value,    "info")) { tl_logger_loglevel(   TL_LOG_LEVEL_INFO); TLSTACKPOP }
            if (tl_char_equals(value,    "warn")) { tl_logger_loglevel(   TL_LOG_LEVEL_WARN); TLSTACKPOP }
            if (tl_char_equals(value,   "error")) { tl_logger_loglevel(  TL_LOG_LEVEL_ERROR); TLSTACKPOP }
            if (tl_char_equals(value,   "fatal")) { tl_logger_loglevel(  TL_LOG_LEVEL_FATAL); TLSTACKPOP }
        }

        if (tl_char_equals(prefix, "engine.graphics.")) {
            if (tl_char_equals(element, "vsync")) {
                global->platform.graphics.vsync = tl_char_equals(value, "true");
                TLTRACE("global->platform.graphics.vsync = %d", global->platform.graphics.vsync)
                TLSTACKPOP
            }
            if (tl_char_equals(element, "wireframe")) {
                global->platform.graphics.wireframe = tl_char_equals(value, "true");
                TLTRACE("global->platform.graphics.wireframe = %d", global->platform.graphics.wireframe)
                TLSTACKPOP
            }
        }

        if (tl_char_equals(prefix, "engine.simulation.") && tl_char_equals(element, "step")) {
            u8 step = strtol(value, (void*)(value + length), 10);
            if (step == 0) {
                TLWARN("Failed to read [%s%s] assuming 24", prefix, value);
                step = 24;
            }

            global->application.simulation.step = 1.0f / (f64) step;
            TLTRACE("global->simulation.step = %f", global->application.simulation.step)
            TLSTACKPOP
        }

        if (tl_char_equals(prefix, "engine.window.")) {
            if (tl_char_equals(element, "title")) {
                global->platform.window.title = tl_string_clone(global->platform.arena, value);
                TLTRACE("global->platform.window.title = %s", value)
                TLSTACKPOP
            }

            if (tl_char_equals(element, "size")) {
                if (tl_char_equals(value,  "SD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_SD; }
                if (tl_char_equals(value,  "HD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_HD; }
                if (tl_char_equals(value, "FHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_FHD; }
                if (tl_char_equals(value, "QHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_QHD; }
                if (tl_char_equals(value, "UHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_UHD; }

                global->platform.window.size.y = (global->platform.window.size.x * 9) / 16;
                TLTRACE("global->platform.window.size = %u x %u", global->platform.window.size.x, global->platform.window.size.y)
                TLSTACKPOP
            }
        }

        TLSTACKPOP
    }

    TLWARN("Unknown prefix: %s", prefix)
    TLSTACKPOP
}

b8 tl_platform_terminate(void) {
    TLSTACKPUSH

    tl_memory_set(&global->platform.graphics, 0, sizeof(global->platform.graphics));
    glfwTerminate();

    TLSTACKPOPV(TRUE)
}