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
    TLVERBOSE("tl_memory_alloc(%d, %s)", size, tl_memory_name(tag))
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

    m_memory_registry->allocated += block_size + size;
    m_memory_registry->tagged_count[TL_MEMORY_BLOCK] += 1;
    m_memory_registry->tagged_size[TL_MEMORY_BLOCK] += block_size;
    // -------------------------------------------
    // Allocate the actual block content
    // -------------------------------------------
    *(block + TL_MEMORY_PAYLOAD) = (u64) __builtin_malloc(size);
    if (*(block + TL_MEMORY_PAYLOAD) == NULL) {
        tl_memory_free( (void*) (block + TL_MEMORY_PAYLOAD));
        return NULL;
    }

    m_memory_registry->allocated += size;
    m_memory_registry->tagged_count[tag] += 1;
    m_memory_registry->tagged_size[tag] += size;
    // -------------------------------------------
    // Return the pointer to the block content
    // -------------------------------------------
    return (void*) (block + TL_MEMORY_PAYLOAD);
}

void tl_memory_free(void *address) {
    TLVERBOSE("tl_memory_free(0x%p)", address);
    u64* block = (u64*)address - TL_MEMORY_PAYLOAD;
    TLVERBOSE("  TL_MEMORY_TAG     %p", (u64*)address - TL_MEMORY_PAYLOAD);
    TLVERBOSE("  TL_MEMORY_SIZE    %p", (u64*)address - TL_MEMORY_SIZE);
    TLVERBOSE("  TL_MEMORY_PAYLOAD %p", (u64*)address);
    // -------------------------------------------
    // Free the actual block content
    // -------------------------------------------
    __builtin_free((void*) *(block + TL_MEMORY_PAYLOAD));
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
}

void tl_memory_set(void *block, i32 value, u64 size) {
    TLVERBOSE("tl_memory_set(0x%p, %d, %llu)", block, value, size);
    __builtin_memset(block, value, size);
}

void tl_memory_copy(void *target, void *source, u64 size) {
    TLVERBOSE("tl_memory_copy(0x%p, 0x%p, %d)", target, source, size);
    __builtin_memcpy(target, source, size);
}
// ########################################################
//                  WINDOW FUNCTIONS
// ########################################################
#include "teleios/window.h"
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>

struct TLWindow {
    const char* title;
    ivec2s size;
    ivec2s position;
    b8 minimized;
    b8 maximized;
    b8 focused;
    b8 hovered;

    Display* x_display;
    xcb_connection_t* xcb_connection;
    xcb_window_t xcb_window;
    xcb_screen_t* xcb_screen;
    xcb_atom_t xcb_protocols;
    xcb_atom_t xcb_delete_win;
};

TLWindow* tl_window_create(u32 width, u32 heigth, const char *title) {
    TLVERBOSE("tl_window_create(%u, %u, %s)", width, heigth, title)
    TLWindow* window = tl_memory_alloc(sizeof(TLWindow), TL_MEMORY_WINDOW);
    window->size.x = width;
    window->size.y = heigth;
    window->title = title;

    window->x_display = XOpenDisplay(NULL);
    XAutoRepeatOff(window->x_display);
    window->xcb_connection = XGetXCBConnection(window->x_display);

    if (xcb_connection_has_error(window->xcb_connection)) {
        TLERROR("Failed to connect to X server via XCB.");
        return NULL;
    }

    return window;
}

void tl_window_destroy(TLWindow *window) {
    TLVERBOSE("tl_window_destroy(%p)", window)
    XAutoRepeatOn(window->x_display);
    // xcb_destroy_window(window->xcb_connection, window->xcb_window);
    XCloseDisplay(window->x_display);
    tl_memory_free(window);
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