#include "teleios/core.h"
#include "teleios/globals.h"

static PFN_handler subscribers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

static const char* tl_event_name(const TLEventCodes code) {
    TLSTACKPUSHA("%d", code)
    switch (code) {
        default                             : TLSTACKPOPV(tl_string(tl_string_from_i32(global->platform.arena, code, 10)));
        case TL_EVENT_WINDOW_CREATED        : TLSTACKPOPV("TL_EVENT_WINDOW_CREATED")
        case TL_EVENT_WINDOW_RESIZED        : TLSTACKPOPV("TL_EVENT_WINDOW_RESIZED")
        case TL_EVENT_WINDOW_CLOSED         : TLSTACKPOPV("TL_EVENT_WINDOW_CLOSED")
        case TL_EVENT_WINDOW_MOVED          : TLSTACKPOPV("TL_EVENT_WINDOW_MOVED")
        case TL_EVENT_WINDOW_MINIMIZED      : TLSTACKPOPV("TL_EVENT_WINDOW_MINIMIZED")
        case TL_EVENT_WINDOW_MAXIMIZED      : TLSTACKPOPV("TL_EVENT_WINDOW_MAXIMIZED")
        case TL_EVENT_WINDOW_RESTORED       : TLSTACKPOPV("TL_EVENT_WINDOW_RESTORED")
        case TL_EVENT_WINDOW_FOCUS_GAINED   : TLSTACKPOPV("TL_EVENT_WINDOW_FOCUS_GAINED")
        case TL_EVENT_WINDOW_FOCUS_LOST     : TLSTACKPOPV("TL_EVENT_WINDOW_FOCUS_LOST")
        case TL_EVENT_MAXIMUM               : TLSTACKPOPV("TL_EVENT_MAXIMUM")
    }
}

b8 tl_event_subscribe(const u16 event, const PFN_handler handler) {
    TLSTACKPUSHA("%u, 0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TLSTACKPOPV(FALSE)
    }

    if (subscribers[event][U8_MAX - 1] != NULL) {
        TLWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        TLSTACKPOPV(FALSE)
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] == NULL) {
            TLTRACE("Subscribing to %s handler function 0x%p", tl_event_name(event), handler)
            TLTRACE("Subscribing to %s has %d handlers", tl_event_name(event), i + 1)
            subscribers[event][i] = handler;
            break;
        }
    }

    TLSTACKPOPV(TRUE)
}

void tl_event_submit(const u16 event, const TLEvent* data) {
    TLSTACKPUSHA("%u, 0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TLSTACKPOP
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] != NULL) {
            const TLEventStatus status = (*subscribers[event][i])(data);
            if (status == TL_EVENT_CONSUMED) {
                break;
            }
        }
    }

    TLSTACKPOP
}