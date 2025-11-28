#ifndef __TELEIOS_GRAPHICS_EVENT__
#define __TELEIOS_GRAPHICS_EVENT__

#include "teleios/teleios.h"
#include "glad/glad.h"
#include "teleios/graphics.h"

static void tl_graphics_resize_viewport(const u8 argc, void** argv) {
    (void) argc; // Unsued
    TLEvent* event = (TLEvent*) argv[0];
    glViewport(0, 0, event->i32[0], event->i32[1]);
}

static TLEventStatus tl_graphics_handle_window_resized(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    tl_graphics_submit_vwa(false, tl_graphics_resize_viewport, 1, &event);
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

#endif