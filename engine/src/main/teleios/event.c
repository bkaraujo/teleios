#include "teleios/core.h"

static PFN_handler subscribers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

static const char* tl_event_name(const TLEventCodes code) {
    TL_STACK_PUSHA("%d", code)
    switch (code) {
        default                             : {
                u16 digits = tl_number_i32_digits(code);
                char buffer[digits + 1]; tl_char_from_i32(buffer, code, 10);
                TL_STACK_POPV(buffer);
        }
        case TL_EVENT_WINDOW_CREATED        : TL_STACK_POPV("TL_EVENT_WINDOW_CREATED")
        case TL_EVENT_WINDOW_RESIZED        : TL_STACK_POPV("TL_EVENT_WINDOW_RESIZED")
        case TL_EVENT_WINDOW_CLOSED         : TL_STACK_POPV("TL_EVENT_WINDOW_CLOSED")
        case TL_EVENT_WINDOW_MOVED          : TL_STACK_POPV("TL_EVENT_WINDOW_MOVED")
        case TL_EVENT_WINDOW_MINIMIZED      : TL_STACK_POPV("TL_EVENT_WINDOW_MINIMIZED")
        case TL_EVENT_WINDOW_MAXIMIZED      : TL_STACK_POPV("TL_EVENT_WINDOW_MAXIMIZED")
        case TL_EVENT_WINDOW_RESTORED       : TL_STACK_POPV("TL_EVENT_WINDOW_RESTORED")
        case TL_EVENT_WINDOW_FOCUS_GAINED   : TL_STACK_POPV("TL_EVENT_WINDOW_FOCUS_GAINED")
        case TL_EVENT_WINDOW_FOCUS_LOST     : TL_STACK_POPV("TL_EVENT_WINDOW_FOCUS_LOST")
        case TL_EVENT_MAXIMUM               : TL_STACK_POPV("TL_EVENT_MAXIMUM")
    }
}

b8 tl_event_subscribe(const u16 event, const PFN_handler handler) {
    TL_STACK_PUSHA("%u, 0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TL_STACK_POPV(false)
    }

    if (subscribers[event][U8_MAX - 1] != NULL) {
        TLWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        TL_STACK_POPV(false)
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] == NULL) {
            TLTRACE("Subscribing to %s handler function 0x%p", tl_event_name(event), handler)
            TLTRACE("Subscribing to %s has %d handlers", tl_event_name(event), i + 1)
            subscribers[event][i] = handler;
            break;
        }
    }

    TL_STACK_POPV(true)
}

void tl_event_submit(const u16 event, const TLEvent* data) {
    TL_STACK_PUSHA("%u, 0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Event type beyond %d", TL_EVENT_MAXIMUM);
        TL_STACK_POP
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] != NULL) {
            const TLEventStatus status = (*subscribers[event][i])(data);
            if (status == TL_EVENT_CONSUMED) {
                break;
            }
        }
    }

    TL_STACK_POP
}