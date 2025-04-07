#include "runtime/memory.h"
#include "teleios/core.h"
#include "teleios/globals.h"
static PFN_handler subscribers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

static const char* tl_event_name(const TLEventCodes code) {
    BKS_STACK_PUSHA("%d", code)
    switch (code) {
        default                             : {
                u16 digits = tl_number_i32_digits(code);
                char* buffer = tl_memory_alloc(global->arena, digits + 1, TL_MEMORY_STRING);
                tl_char_from_i32(buffer, code, 10);
                BKS_STACK_POPV(buffer);
        }
        case TL_EVENT_WINDOW_CREATED        : BKS_STACK_POPV("TL_EVENT_WINDOW_CREATED")
        case TL_EVENT_WINDOW_RESIZED        : BKS_STACK_POPV("TL_EVENT_WINDOW_RESIZED")
        case TL_EVENT_WINDOW_CLOSED         : BKS_STACK_POPV("TL_EVENT_WINDOW_CLOSED")
        case TL_EVENT_WINDOW_MOVED          : BKS_STACK_POPV("TL_EVENT_WINDOW_MOVED")
        case TL_EVENT_WINDOW_MINIMIZED      : BKS_STACK_POPV("TL_EVENT_WINDOW_MINIMIZED")
        case TL_EVENT_WINDOW_MAXIMIZED      : BKS_STACK_POPV("TL_EVENT_WINDOW_MAXIMIZED")
        case TL_EVENT_WINDOW_RESTORED       : BKS_STACK_POPV("TL_EVENT_WINDOW_RESTORED")
        case TL_EVENT_WINDOW_FOCUS_GAINED   : BKS_STACK_POPV("TL_EVENT_WINDOW_FOCUS_GAINED")
        case TL_EVENT_WINDOW_FOCUS_LOST     : BKS_STACK_POPV("TL_EVENT_WINDOW_FOCUS_LOST")
        case TL_EVENT_MAXIMUM               : BKS_STACK_POPV("TL_EVENT_MAXIMUM")
    }
}

b8 tl_event_subscribe(const u16 event, const PFN_handler handler) {
    BKS_STACK_PUSHA("%u, 0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        BKSWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        BKS_STACK_POPV(false)
    }

    if (subscribers[event][U8_MAX - 1] != NULL) {
        BKSWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        BKS_STACK_POPV(false)
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] == NULL) {
            BKSTRACE("Subscribing to %s handler function 0x%p", tl_event_name(event), handler)
            BKSTRACE("Subscribing to %s has %d handlers", tl_event_name(event), i + 1)
            subscribers[event][i] = handler;
            break;
        }
    }

    BKS_STACK_POPV(true)
}

void tl_event_submit(const u16 event, const TLEvent* data) {
    BKS_STACK_PUSHA("%u, 0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        BKSWARN("Event type beyond %d", TL_EVENT_MAXIMUM);
        BKS_STACK_POP
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] != NULL) {
            const TLEventStatus status = (*subscribers[event][i])(data);
            if (status == TL_EVENT_CONSUMED) {
                break;
            }
        }
    }

    BKS_STACK_POP
}