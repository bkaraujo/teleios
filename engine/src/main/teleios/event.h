#ifndef __TELEIOS_EVENT__
#define __TELEIOS_EVENT__

#include "teleios/defines.h"

/**
 * @brief Window and input event type codes
 *
 * Enumerates all predefined event types in the system.
 * Custom events can be defined by adding constants after TL_EVENT_MAXIMUM.
 *
 * @note Window events are prefixed with TL_EVENT_WINDOW_
 * @note Input events are prefixed with TL_EVENT_INPUT_
 * @note TL_EVENT_MAXIMUM marks the end of predefined events
 *
 * @see tl_event_subscribe
 * @see tl_event_submit
 */
typedef enum {
    TL_EVENT_WINDOW_CREATED,        ///< Window successfully created

    /**
     * Window resized event
     * @details event.i32[0] = new width in pixels
     * @details event.i32[1] = new height in pixels
     */
    TL_EVENT_WINDOW_RESIZED,

    TL_EVENT_WINDOW_CLOSED,         ///< Window close button pressed or system close request

    /**
     * Window moved event
     * @details event.i32[0] = new x position in screen coordinates
     * @details event.i32[1] = new y position in screen coordinates
     */
    TL_EVENT_WINDOW_MOVED,

    TL_EVENT_WINDOW_MINIMIZED,      ///< Window minimized by user
    TL_EVENT_WINDOW_MAXIMIZED,      ///< Window maximized by user
    TL_EVENT_WINDOW_RESTORED,       ///< Window restored from minimized/maximized state
    TL_EVENT_WINDOW_FOCUS_GAINED,   ///< Window gained input focus (became active)
    TL_EVENT_WINDOW_FOCUS_LOST,     ///< Window lost input focus (no longer active)

    TL_EVENT_INPUT_KEY_PRESSED,     ///< Keyboard key pressed
    TL_EVENT_INPUT_KEY_RELEASED,    ///< Keyboard key released
    TL_EVENT_INPUT_CURSOR_PRESSED,  ///< Mouse button pressed
    TL_EVENT_INPUT_CURSOR_RELEASED, ///< Mouse button released
    TL_EVENT_INPUT_CURSOR_MOVED,    ///< Mouse cursor position changed
    TL_EVENT_INPUT_CURSOR_SCROLLED, ///< Mouse wheel scrolled
    TL_EVENT_INPUT_CURSOR_ENTERED,  ///< Mouse cursor entered window
    TL_EVENT_INPUT_CURSOR_EXITED,   ///< Mouse cursor exited window

    TL_EVENT_MAXIMUM                ///< Sentinel value marking end of predefined events
} TLEventCodes;

/**
 * @brief Event data union for flexible event payload storage
 *
 * Allows event data to be stored in multiple formats. Use the format
 * appropriate for your event type. For example:
 * - Use i32[0] and i32[1] for WINDOW_RESIZED (width and height)
 * - Use i32[0] for key codes in INPUT_KEY_PRESSED
 * - Use f32[0] and f32[1] for mouse position in INPUT_CURSOR_MOVED
 *
 * @see TLEventCodes - Event type enum with data layout hints
 */
typedef union {
    i64 i64[2];                     ///< Two signed 64-bit integers
    u64 u64[2];                     ///< Two unsigned 64-bit integers
    f64 f64[2];                     ///< Two 64-bit floats

    i32 i32[4];                     ///< Four signed 32-bit integers
    u32 u32[4];                     ///< Four unsigned 32-bit integers
    f32 f32[4];                     ///< Four 32-bit floats

    i16 i16[8];                     ///< Eight signed 16-bit integers
    u16 u16[8];                     ///< Eight unsigned 16-bit integers

    i8 i8[16];                      ///< Sixteen signed 8-bit integers
    u8 u8[16];                      ///< Sixteen unsigned 8-bit integers
} TLEvent;

/**
 * @brief Event handler return status
 *
 * Indicates whether an event handler consumed the event (stopping propagation)
 * or allows propagation to other handlers.
 *
 * @see TLEventHandler - Event handler function type
 */
typedef enum {
    TL_EVENT_CONSUMED    = 10,      ///< Handler consumed event - stop propagation
    TL_EVENT_AVAILABLE   = 11,      ///< Event available for next handler - continue
} TLEventStatus;

/**
 * @brief Event handler function pointer type
 *
 * Event handlers have this signature. They receive the event data and
 * return a TLEventStatus indicating whether propagation should continue.
 *
 * @see tl_event_subscribe
 *
 * @example
 * TLEventStatus my_handler(const TLEvent* event) {
 *     // Handle event
 *     return TL_EVENT_CONSUMED;  // or TL_EVENT_AVAILABLE
 * }
 *
 * tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, my_handler);
 */
typedef TLEventStatus (*TLEventHandler)(const TLEvent*);

/**
 * @brief Subscribe a handler function to an event type
 *
 * Registers an event handler to be called when the specified event is submitted.
 * Multiple handlers can be subscribed to the same event; they are called in
 * subscription order.
 *
 * @param event Event code to subscribe to (see TLEventCodes)
 * @param handler Function pointer to call when event is submitted
 * @return true if handler registered successfully, false on failure
 *         (e.g., maximum handlers reached for this event)
 *
 * @note Maximum 255 handlers can be subscribed per event type
 * @note Handlers are called in subscription order
 * @note Handler returning TL_EVENT_CONSUMED stops further propagation
 * @note Safe to call from event handlers (handlers are called immediately)
 *
 * @see tl_event_submit
 * @see TLEventHandler
 * @see TLEventCodes
 *
 * @code
 * // Handler function
 * TLEventStatus on_close(const TLEvent* event) {
 *     TLINFO("Window closing");
 *     return TL_EVENT_CONSUMED;
 * }
 *
 * // Subscribe to window close event
 * if (!tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, on_close)) {
 *     TLERROR("Failed to subscribe to window close event");
 * }
 * @endcode
 */
b8 tl_event_subscribe(u16 event, TLEventHandler handler);

/**
 * @brief Submit an event for immediate processing
 *
 * Dispatches an event to all registered handlers. Handlers are called
 * synchronously in subscription order. If a handler returns TL_EVENT_CONSUMED,
 * remaining handlers are not called.
 *
 * @param event Event code to submit (see TLEventCodes or custom codes)
 * @param data Optional event data. Can be NULL for events with no data.
 *             Event data should be filled according to the event type.
 *
 * @note Event data is copied internally - do not keep pointers to it
 * @note Handlers are called immediately and synchronously
 * @note Window and input events are submitted automatically by GLFW callbacks
 *
 * @see tl_event_subscribe
 * @see TLEventCodes - Event types with data layout documentation
 *
 * @code
 * // Submit event with no data
 * tl_event_submit(TL_EVENT_WINDOW_CREATED, NULL);
 *
 * // Submit event with data
 * TLEvent event = {0};
 * event.i32[0] = 1920;  // width
 * event.i32[1] = 1080;  // height
 * tl_event_submit(TL_EVENT_WINDOW_RESIZED, &event);
 * @endcode
 */
void tl_event_submit(u16 event, const TLEvent* data);

#endif
