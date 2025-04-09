#include "teleios/core.h"
#include "teleios/globals.h"
static PFN_handler subscribers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

static const char* tl_event_name(const TLEventCodes code) {
    K_FRAME_PUSH_WITH("%d", code)
    switch (code) {
        default                             : {
                u16 digits = tl_number_i32_digits(code);
                char* buffer = k_memory_allocator_alloc(global->allocator, digits + 1, TL_MEMORY_STRING);
                tl_char_from_i32(buffer, code, 10);
                K_FRAME_POP_WITH(buffer);
        }
        case TL_EVENT_WINDOW_CREATED        : K_FRAME_POP_WITH("TL_EVENT_WINDOW_CREATED")
        case TL_EVENT_WINDOW_RESIZED        : K_FRAME_POP_WITH("TL_EVENT_WINDOW_RESIZED")
        case TL_EVENT_WINDOW_CLOSED         : K_FRAME_POP_WITH("TL_EVENT_WINDOW_CLOSED")
        case TL_EVENT_WINDOW_MOVED          : K_FRAME_POP_WITH("TL_EVENT_WINDOW_MOVED")
        case TL_EVENT_WINDOW_MINIMIZED      : K_FRAME_POP_WITH("TL_EVENT_WINDOW_MINIMIZED")
        case TL_EVENT_WINDOW_MAXIMIZED      : K_FRAME_POP_WITH("TL_EVENT_WINDOW_MAXIMIZED")
        case TL_EVENT_WINDOW_RESTORED       : K_FRAME_POP_WITH("TL_EVENT_WINDOW_RESTORED")
        case TL_EVENT_WINDOW_FOCUS_GAINED   : K_FRAME_POP_WITH("TL_EVENT_WINDOW_FOCUS_GAINED")
        case TL_EVENT_WINDOW_FOCUS_LOST     : K_FRAME_POP_WITH("TL_EVENT_WINDOW_FOCUS_LOST")
        case TL_EVENT_MAXIMUM               : K_FRAME_POP_WITH("TL_EVENT_MAXIMUM")
    }
}

b8 tl_event_subscribe(const u16 event, const PFN_handler handler) {
    K_FRAME_PUSH_WITH("%u, 0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        KWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        K_FRAME_POP_WITH(false)
    }

    if (subscribers[event][U8_MAX - 1] != NULL) {
        KWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        K_FRAME_POP_WITH(false)
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] == NULL) {
            KTRACE("Subscribing to %s handler function 0x%p", tl_event_name(event), handler)
            KTRACE("Subscribing to %s has %d handlers", tl_event_name(event), i + 1)
            subscribers[event][i] = handler;
            break;
        }
    }

    K_FRAME_POP_WITH(true)
}

void tl_event_submit(const u16 event, const TLEvent* data) {
    K_FRAME_PUSH_WITH("%u, 0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        KWARN("Event type beyond %d", TL_EVENT_MAXIMUM);
        K_FRAME_POP
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] != NULL) {
            const TLEventStatus status = (*subscribers[event][i])(data);
            if (status == TL_EVENT_CONSUMED) {
                break;
            }
        }
    }

    K_FRAME_POP
}