#include "teleios/core.h"

static const PFN_handler* subscribers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

b8 tl_event_subscribe(const u16 event, const PFN_handler handler) {
    TLSTACKPUSHA("%u, 0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TLSTACKPOP
        return FALSE;
    }

    if (subscribers[event][U8_MAX - 1] != NULL) {
        TLWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        TLSTACKPOP
        return FALSE;
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (subscribers[event][i] == NULL) {
            subscribers[event][i] = &handler;
            break;
        }
    }

    TLSTACKPOP
    return TRUE;
}

void tl_event_submit(const u16 event, TLEvent* data) {
    TLSTACKPUSHA("%u, 0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TLSTACKPOP
        return;
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