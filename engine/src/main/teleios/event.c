#include "teleios/teleios.h"

static TLEventHandler m_handlers[TL_EVENT_MAXIMUM][U8_MAX] = { 0 };

static const char* tl_event_name(const TLEventCodes code) {
    TL_PROFILER_PUSH_WITH("%d", code)

    switch (code) {
        case TL_EVENT_WINDOW_CREATED        : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_CREATED")
        case TL_EVENT_WINDOW_RESIZED        : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_RESIZED")
        case TL_EVENT_WINDOW_CLOSED         : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_CLOSED")
        case TL_EVENT_WINDOW_MOVED          : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_MOVED")
        case TL_EVENT_WINDOW_MINIMIZED      : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_MINIMIZED")
        case TL_EVENT_WINDOW_MAXIMIZED      : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_MAXIMIZED")
        case TL_EVENT_WINDOW_RESTORED       : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_RESTORED")
        case TL_EVENT_WINDOW_FOCUS_GAINED   : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_FOCUS_GAINED")
        case TL_EVENT_WINDOW_FOCUS_LOST     : TL_PROFILER_POP_WITH("TL_EVENT_WINDOW_FOCUS_LOST")


        case TL_EVENT_INPUT_KEY_PRESSED     : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_KEY_PRESSED")
        case TL_EVENT_INPUT_KEY_RELEASED    : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_KEY_RELEASED")
        case TL_EVENT_INPUT_CURSOR_PRESSED  : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_CURSOR_PRESSED")
        case TL_EVENT_INPUT_CURSOR_RELEASED : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_CURSOR_RELEASED")
        case TL_EVENT_INPUT_CURSOR_MOVED    : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_CURSOR_MOVED")
        case TL_EVENT_INPUT_CURSOR_SCROLLED : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_CURSOR_SCROLLED")
        case TL_EVENT_INPUT_CURSOR_ENTERED  : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_CURSOR_ENTERED")
        case TL_EVENT_INPUT_CURSOR_EXITED   : TL_PROFILER_POP_WITH("TL_EVENT_INPUT_CURSOR_EXITED")

        case TL_EVENT_MAXIMUM               : TL_PROFILER_POP_WITH("TL_EVENT_MAXIMUM")
    }

    const TLString* string = tl_number_i32_to_char(global->allocator, code, 10);
    const char* cstr = tl_string_cstr(string);

    TL_PROFILER_POP_WITH(cstr);
}

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

    for (u8 i = 0; i < U8_MAX; ++i) {
        if (m_handlers[event][i] != NULL) {
            const TLEventStatus status = (*m_handlers[event][i])(data);
            if (status == TL_EVENT_CONSUMED) {
                break;
            }
        }
    }

    TL_PROFILER_POP
}