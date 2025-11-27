#ifndef __TELEIOS_APPLICATION_EVENT__
#define __TELEIOS_APPLICATION_EVENT__

#include "teleios/teleios.h"

static TLEventStatus tl_application_handle_window_closed(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    global->running = false;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_application_handle_window_restored(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    global->suspended = false;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_application_handle_window_minimized(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    global->suspended = true;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

#endif