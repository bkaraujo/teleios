#ifndef __TELEIOS_EVENT__
#define __TELEIOS_EVENT__

#include "teleios/defines.h"

typedef enum {
    TL_EVENT_WINDOW_CREATED,
    /**
     *    event.i32[0] = width;<br>
     *    event.i32[1] = height;
     */
    TL_EVENT_WINDOW_RESIZED,
    /** No aditional data */
    TL_EVENT_WINDOW_CLOSED,
    /**
     *    event.i32[0] = xPos;<br>
     *    event.i32[1] = yPos;
     */
    TL_EVENT_WINDOW_MOVED,
    /** No aditional data */
    TL_EVENT_WINDOW_MINIMIZED,
    /** No aditional data */
    TL_EVENT_WINDOW_MAXIMIZED,
    /** No aditional data */
    TL_EVENT_WINDOW_RESTORED,
    /** No aditional data */
    TL_EVENT_WINDOW_FOCUS_GAINED,
    /** No aditional data */
    TL_EVENT_WINDOW_FOCUS_LOST,

    TL_EVENT_INPUT_KEY_PRESSED,
    TL_EVENT_INPUT_KEY_RELEASED,
    TL_EVENT_INPUT_CURSOR_PRESSED,
    TL_EVENT_INPUT_CURSOR_RELEASED,
    TL_EVENT_INPUT_CURSOR_MOVED,
    TL_EVENT_INPUT_CURSOR_SCROLLED,
    TL_EVENT_INPUT_CURSOR_ENTERED,
    TL_EVENT_INPUT_CURSOR_EXITED,

    TL_EVENT_MAXIMUM
} TLEventCodes;

typedef union {
    i64 i64[2];
    u64 u64[2];
    f64 f64[2];

    i32 i32[4];
    u32 u32[4];
    f32 f32[4];

    i16 i16[8];
    u16 u16[8];

    i8 i8[16];
    u8 u8[16];
} TLEvent;

typedef enum {
    TL_EVENT_CONSUMED    = 10,
    TL_EVENT_AVAILABLE   = 11,
} TLEventStatus;

typedef TLEventStatus (*TLEventHandler)(const TLEvent*);

/**
 * @brief Subscribe for notification on occurrences of <code>event</code>
 * @param event The event code
 * @param handler The function that will handle the event
 * @return <code>true</code> if the handler were registered successfully, <code>false</code> otherwise
 */
b8 tl_event_subscribe(u16 event, TLEventHandler handler);

/**
 * @brief Submit a new event for immediate processing.
 *
 * @param event The event code
 * @param data <code>optional</code> event data
 */
void tl_event_submit(u16 event, const TLEvent* data);

#endif