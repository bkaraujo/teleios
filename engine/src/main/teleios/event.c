#include "teleios/teleios.h"

static TLEventHandler m_handlers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };
static u8 m_handler_count[TL_EVENT_MAXIMUM] = { 0 };

b8 tl_event_subscribe(const u16 event, const TLEventHandler handler) {
    TL_PROFILER_PUSH_WITH("%u, %0x%p", event, handler)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Eventy type beyond %d", TL_EVENT_MAXIMUM);
        TL_PROFILER_POP_WITH(false)
    }

    if (m_handlers[event][U8_MAX - 1] != NULL) {
        TLWARN("Event %u reached maximum of %d handlers", event, U8_MAX - 1);
        TL_PROFILER_POP_WITH(false)
    }

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (m_handlers[event][i] == NULL) {
            m_handlers[event][i] = handler;
            m_handler_count[event]++;
            break;
        }
    }

    TL_PROFILER_POP_WITH(true)
}

void tl_event_submit(const u16 event, const TLEvent* data) {
    TL_PROFILER_PUSH_WITH("%u, %0x%p", event, data)

    if (event >= TL_EVENT_MAXIMUM) {
        TLWARN("Event type beyond %d", TL_EVENT_MAXIMUM);
        TL_PROFILER_POP
    }

    const u8 count = m_handler_count[event];
    for (u8 i = 0; i < count; ++i) {
        const TLEventStatus status = (*m_handlers[event][i])(data);
        if (status == TL_EVENT_CONSUMED) {
            break;
        }
    }

    TL_PROFILER_POP
}