#include "teleios/platform_detector.h"
#ifdef TLPLATFORM_LINUX

#include <stdio.h>
#include <malloc.h>

#define SECS_IN_DAY (24 * 60 * 60)
// ########################################################
//                     CONSOLE FUNCTIONS
// ########################################################
#include "teleios/console.h"
void tl_console_stdout(TLLogLevel level, const char *message) {
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE, VERBOSE
    static const char *strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30", "1;29"};
    fprintf(stdout, "\033[%sm%s\033[0m", strings[level], message);
}
// ########################################################
//                    TIME FUNCTIONS
// ########################################################
#define __USE_POSIX 1
#define __USE_POSIX199309 1
#include <time.h>
#include "teleios/time.h"

void tl_time_clock(TLClock* clock) {
    struct timespec now = { 0 };
    if (clock_gettime(CLOCK_REALTIME, &now) != 0) return;

    struct tm localtime = { 0 };
    if (localtime_r(&now.tv_sec, &localtime) == NULL) return;

    clock->year = localtime.tm_year + 1900;
    clock->month = localtime.tm_mon + 1;
    clock->day = localtime.tm_mday;
    clock->hour = localtime.tm_hour;
    clock->minute = localtime.tm_min;
    clock->second = localtime.tm_sec;
    clock->millis = now.tv_nsec / 1000;
}
// ########################################################
//                    MEMORY FUNCTIONS
// 
// __builtin_malloc - GCC is free to eliminate calls if its result is never used, because there are no additional side-effects.
// __builtin_free   - GCC is free to eliminate calls if its result is never used, because there are no additional side-effects.
// ########################################################
#include "teleios/memory.h"

typedef struct {
    u64 allocated;
    u64 tagged_count[TL_MEMORY_MAXIMUM];
    u64 tagged_size[TL_MEMORY_MAXIMUM];
} TLMemoryRegistry;
enum {

    TL_MEMORY_TAG,
    TL_MEMORY_SIZE,
    TL_MEMORY_PAYLOAD,
    TL_MEMORY_LENGTH
} TLMemoryBLock;

static TLMemoryRegistry* m_memory_registry;

static const char *tl_memory_name(TLMemoryTag tag) {
    switch (tag) {
        case TL_MEMORY_BLOCK: return "TL_MEMORY_BLOCK";
        case TL_MEMORY_CONTAINER_LIST: return "TL_MEMORY_CONTAINER_LIST";
        case TL_MEMORY_CONTAINER_NODE: return "TL_MEMORY_CONTAINER_NODE";
        case TL_MEMORY_WINDOW: return "TL_MEMORY_WINDOW";
        case TL_MEMORY_MAXIMUM: return "TL_MEMORY_MAXIMUM";
    }

    return "???";
}

void *tl_memory_alloc(u64 size, TLMemoryTag tag) {
    TLTRACE(">> tl_memory_alloc(%d, %s)", size, tl_memory_name(tag))
    static u64 block_size = TL_MEMORY_LENGTH * sizeof(u64);
    // -------------------------------------------
    // Allocate the block structure
    // -------------------------------------------
    u64* block = (u64*) __builtin_malloc(block_size);
    if (block == NULL) { return NULL; }
    __builtin_memset((void*) block, 0, block_size);
    
    *(block + TL_MEMORY_TAG) = tag;
    *(block + TL_MEMORY_SIZE) = size;

    TLVERBOSE("  TL_MEMORY_TAG     %p", block + TL_MEMORY_TAG);
    TLVERBOSE("  TL_MEMORY_SIZE    %p", block + TL_MEMORY_SIZE);
    TLVERBOSE("  TL_MEMORY_PAYLOAD %p", block + TL_MEMORY_PAYLOAD);

    m_memory_registry->allocated += block_size;
    m_memory_registry->tagged_count[TL_MEMORY_BLOCK] += 1;
    m_memory_registry->tagged_size[TL_MEMORY_BLOCK] += block_size;
    // -------------------------------------------
    // Allocate the actual block content
    // -------------------------------------------
    *(block + TL_MEMORY_PAYLOAD) = (u64) __builtin_malloc(size);
    if ((block + TL_MEMORY_PAYLOAD) == NULL) {
        tl_memory_free( (void*) (block + TL_MEMORY_PAYLOAD));
        return NULL;
    }
    
    __builtin_memset((void*) *(block + TL_MEMORY_PAYLOAD), 0, size);

    m_memory_registry->allocated += size;
    m_memory_registry->tagged_count[tag] += 1;
    m_memory_registry->tagged_size[tag] += size;
    // -------------------------------------------
    // Return the pointer to the block content
    // -------------------------------------------
    TLTRACE("<< tl_memory_alloc(%d, %s)", size, tl_memory_name(tag))
    return (void*) (block + TL_MEMORY_PAYLOAD);
}

void tl_memory_free(void *address) {
    TLTRACE(">> tl_memory_free(0x%p)", address);
    u64* block = (u64*)address - TL_MEMORY_PAYLOAD;
    TLVERBOSE("  TL_MEMORY_TAG     %p", (u64*)address - TL_MEMORY_PAYLOAD);
    TLVERBOSE("  TL_MEMORY_SIZE    %p", (u64*)address - TL_MEMORY_SIZE);
    TLVERBOSE("  TL_MEMORY_PAYLOAD %p", (u64*)address);
    // -------------------------------------------
    // Free the actual block content
    // -------------------------------------------
    m_memory_registry->allocated -= *(block + TL_MEMORY_SIZE);
    m_memory_registry->tagged_count[*(block + TL_MEMORY_TAG)] -= 1;
    m_memory_registry->tagged_size[*(block + TL_MEMORY_TAG)] -= *(block + TL_MEMORY_SIZE);
    // -------------------------------------------
    // Free the block structure
    // -------------------------------------------
    __builtin_free((void*) block);
    static u64 block_size = TL_MEMORY_LENGTH * sizeof(u64);
    m_memory_registry->allocated -= block_size;
    m_memory_registry->tagged_count[TL_MEMORY_BLOCK] -= 1;
    m_memory_registry->tagged_size[TL_MEMORY_BLOCK] -= block_size;
    TLTRACE("<< tl_memory_free(0x%p)", address);
}

void tl_memory_set(void *block, i32 value, u64 size) {
    TLTRACE(">> tl_memory_set(0x%p, %d, %llu)", block, value, size);
    __builtin_memset(block, value, size);
    TLTRACE("<< tl_memory_set(0x%p, %d, %llu)", block, value, size);
}

void tl_memory_copy(void *target, void *source, u64 size) {
    TLTRACE(">> tl_memory_copy(0x%p, 0x%p, %d)", target, source, size);
    __builtin_memcpy(target, source, size);
    TLTRACE("<< tl_memory_copy(0x%p, 0x%p, %d)", target, source, size);
}
// ########################################################
//                  WINDOW FUNCTIONS
// ########################################################
#include "teleios/window.h"
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

Display* x_display;

struct TLWindow {
    const char* title;
    ivec2s size;
    ivec2s position;
    b8 minimized;
    b8 maximized;
    b8 focused;
    b8 hovered;

    xcb_connection_t* xcb_connection;
    xcb_void_cookie_t handle;
    xcb_window_t xcb_window;
    xcb_screen_t* xcb_screen;
    xcb_atom_t xcb_protocols;
    xcb_atom_t xcb_delete_win;
};

TLWindow* tl_window_create(u32 width, u32 height, const char *title) {
    TLTRACE(">> tl_window_create(%u, %u, %s)", width, height, title)
    TLWindow* window = tl_memory_alloc(sizeof(TLWindow), TL_MEMORY_WINDOW);
    window->size.x = width;
    window->size.y = height;
    window->title = title;

    x_display = XOpenDisplay(NULL);
    TLVERBOSE("  Auto Repeat: off")
    XAutoRepeatOff(x_display);
    window->xcb_connection = XGetXCBConnection(x_display);

    if (xcb_connection_has_error(window->xcb_connection)) {
        TLERROR("Failed to connect to X server via XCB.");
        return NULL;
    }
    
    const struct xcb_setup_t* setup = xcb_get_setup(window->xcb_connection);

    // ------------------------------------
    // Loop through screens using iterator
    // ------------------------------------
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);

    int screen_p = 0;
    for (i32 s = screen_p; s > 0; --s) { xcb_screen_next(&it); }
    window->xcb_screen = it.data;
    window->xcb_window = xcb_generate_id(window->xcb_connection);


    // Register event types.
    // XCB_CW_BACK_PIXEL = filling then window bg with a single colour
    // XCB_CW_EVENT_MASK is required.
    u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    // Listen for keyboard and mouse buttons
    u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;
    event_values |= XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE;
    event_values |= XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION;
    event_values |= XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    // Values to be sent over XCB (bg colour, events)
    u32 value_list[] = {window->xcb_screen->black_pixel, event_values};

    window->handle = xcb_create_window(
        window->xcb_connection,
        XCB_COPY_FROM_PARENT,               // depth
        window->xcb_window,                 // parent
        window->xcb_screen->root,           // parent
        0,                                  //x
        0,                                  //y
        window->size.x,                     //width
        window->size.y,                     //height
        0,                                  // No border
        XCB_WINDOW_CLASS_INPUT_OUTPUT,      //class
        window->xcb_screen->root_visual,
        event_mask,
        value_list);

    TLVERBOSE("  created %d", window->handle);
    window->xcb_delete_win = xcb_intern_atom_reply(
        window->xcb_connection, 
        xcb_intern_atom(window->xcb_connection, 0, __builtin_strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW"), 
        NULL
    )->atom;

    window->xcb_protocols = xcb_intern_atom_reply(
        window->xcb_connection,
        xcb_intern_atom(window->xcb_connection, 0, __builtin_strlen("WM_PROTOCOLS"), "WM_PROTOCOLS"), 
        NULL
    )->atom;

    xcb_change_property(window->xcb_connection, XCB_PROP_MODE_REPLACE, window->xcb_window, window->xcb_protocols, 4, 32, 1, &window->xcb_delete_win);
    xcb_map_window(window->xcb_connection, window->xcb_window);

    xcb_change_property(window->xcb_connection, XCB_PROP_MODE_REPLACE, window->xcb_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, __builtin_strlen(title), title);
    i32 stream_result = xcb_flush(window->xcb_connection);
    if (stream_result <= 0) {
        TLERROR("Failed to flush X windows: %d", stream_result);
        TLTRACE("<< tl_window_create(%u, %u, %s)", width, height, title)
        return NULL;
    }


    TLTRACE("<< tl_window_create(%u, %u, %s)", width, height, title)
    return window;
}

void tl_window_destroy(TLWindow *window) {
    TLTRACE(">> tl_window_destroy(%p)", window)
    TLVERBOSE("  Auto Repeat: on")
    XAutoRepeatOn(x_display);
    xcb_destroy_window(window->xcb_connection, window->xcb_window);
    // XCloseDisplay(x_display);
    tl_memory_free(window);
    TLTRACE("<< tl_window_destroy(%p)", window)
}
// ########################################################
//                  LIFECYCLE FUNCTIONS
// ########################################################
b8 tl_platform_initialize(void) {
    TLTRACE("tl_platform_initialize(void)")
    m_memory_registry = (TLMemoryRegistry*) __builtin_malloc(sizeof(TLMemoryRegistry));
    __builtin_memset((void*) m_memory_registry, 0, sizeof(TLMemoryRegistry));

    return TRUE;
}

b8 tl_platform_terminate(void) {
    TLTRACE("tl_platform_terminate(void)")
    if (m_memory_registry->allocated != 0) {
        TLWARN("Leaked %llu bytes", m_memory_registry->allocated);
        for (u32 i = 0 ; i < TL_MEMORY_MAXIMUM ; ++i) {
            if (m_memory_registry->tagged_size[i] != 0) {
                TLWARN("  at %-30s: %llu bytes", tl_memory_name(i), m_memory_registry->tagged_size[i]);
            }
        }
    }

    __builtin_free(m_memory_registry);
    return TRUE;
}

#endif