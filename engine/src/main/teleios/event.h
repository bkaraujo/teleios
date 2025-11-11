/**
 * @file event.h
 * @brief Event system for decoupled event-driven communication
 *
 * This module provides an event subscription and dispatch system. It implements
 * the Observer pattern, allowing different parts of the application to subscribe
 * to and respond to events without direct coupling.
 *
 * @section event_types Event Types
 *
 * The system supports two categories of events:
 *
 * **Window Events (automatically submitted by GLFW callbacks):**
 * - TL_EVENT_WINDOW_CREATED - Window creation complete
 * - TL_EVENT_WINDOW_RESIZED - Window resized (data: width, height)
 * - TL_EVENT_WINDOW_CLOSED - Window close requested
 * - TL_EVENT_WINDOW_MOVED - Window position changed (data: xPos, yPos)
 * - TL_EVENT_WINDOW_MINIMIZED - Window minimized
 * - TL_EVENT_WINDOW_MAXIMIZED - Window maximized
 * - TL_EVENT_WINDOW_RESTORED - Window restored from minimized/maximized
 * - TL_EVENT_WINDOW_FOCUS_GAINED - Window gained input focus
 * - TL_EVENT_WINDOW_FOCUS_LOST - Window lost input focus
 *
 * **Input Events (automatically submitted by GLFW callbacks):**
 * - TL_EVENT_INPUT_KEY_PRESSED - Key pressed
 * - TL_EVENT_INPUT_KEY_RELEASED - Key released
 * - TL_EVENT_INPUT_CURSOR_PRESSED - Mouse button pressed
 * - TL_EVENT_INPUT_CURSOR_RELEASED - Mouse button released
 * - TL_EVENT_INPUT_CURSOR_MOVED - Mouse cursor moved
 * - TL_EVENT_INPUT_CURSOR_SCROLLED - Mouse wheel scrolled
 * - TL_EVENT_INPUT_CURSOR_ENTERED - Cursor entered window
 * - TL_EVENT_INPUT_CURSOR_EXITED - Cursor left window
 *
 * @section event_data Event Data Union
 *
 * Event data is stored in a union (TLEvent) that can hold multiple data formats:
 * - i64[2] / u64[2] / f64[2] - Two 64-bit values
 * - i32[4] / u32[4] / f32[4] - Four 32-bit values
 * - i16[8] / u16[8] - Eight 16-bit values
 * - i8[16] / u8[16] - Sixteen 8-bit values
 *
 * Window events populate specific fields:
 * - WINDOW_RESIZED: event.i32[0] = width, event.i32[1] = height
 * - WINDOW_MOVED: event.i32[0] = xPos, event.i32[1] = yPos
 *
 * @section handler_callbacks Event Handlers
 *
 * Event handlers are function pointers with this signature:
 * ```c
 * TLEventStatus handler(const TLEvent* event)
 * ```
 *
 * Handlers return:
 * - TL_EVENT_CONSUMED (10) - Stop propagation to other handlers
 * - TL_EVENT_AVAILABLE (11) - Continue to next handler
 *
 * Up to 255 handlers can be subscribed to each event type.
 *
 * @section usage Usage Examples
 *
 * **Subscribing to window events:**
 * @code
 * #include "teleios/event.h"
 * #include "teleios/logger.h"
 *
 * // Handler for window close
 * TLEventStatus on_window_closed(const TLEvent* event) {
 *     TLINFO("Window close requested");
 *     // Return CONSUMED to stop propagation
 *     return TL_EVENT_CONSUMED;
 * }
 *
 * // In initialization:
 * tl_event_subscribe(TL_EVENT_WINDOW_CLOSED, on_window_closed);
 * @endcode
 *
 * **Handling window resize:**
 * @code
 * TLEventStatus on_window_resized(const TLEvent* event) {
 *     i32 width = event->i32[0];
 *     i32 height = event->i32[1];
 *
 *     TLDEBUG("Window resized to %d x %d", width, height);
 *
 *     // Update viewport and camera
 *     update_viewport(width, height);
 *     update_camera_aspect((f32)width / (f32)height);
 *
 *     return TL_EVENT_AVAILABLE;  // Allow other handlers
 * }
 *
 * tl_event_subscribe(TL_EVENT_WINDOW_RESIZED, on_window_resized);
 * @endcode
 *
 * **Submitting custom events:**
 * @code
 * // Define custom event code
 * #define MY_CUSTOM_EVENT (TL_EVENT_MAXIMUM + 1)
 *
 * // Submit event with custom data
 * TLEvent event = {0};
 * event.i32[0] = 42;
 * event.i32[1] = 100;
 *
 * tl_event_submit(MY_CUSTOM_EVENT, &event);
 *
 * // Handler for custom event
 * TLEventStatus on_custom_event(const TLEvent* event) {
 *     i32 value1 = event->i32[0];
 *     i32 value2 = event->i32[1];
 *     TLINFO("Custom event: %d, %d", value1, value2);
 *     return TL_EVENT_AVAILABLE;
 * }
 *
 * tl_event_subscribe(MY_CUSTOM_EVENT, on_custom_event);
 * @endcode
 *
 * **Handling input events:**
 * @code
 * TLEventStatus on_key_pressed(const TLEvent* event) {
 *     // Key code typically in event.i32[0]
 *     i32 key_code = event->i32[0];
 *
 *     if (key_code == GLFW_KEY_ESCAPE) {
 *         TLINFO("ESC pressed - exiting");
 *         return TL_EVENT_CONSUMED;
 *     }
 *
 *     return TL_EVENT_AVAILABLE;
 * }
 *
 * tl_event_subscribe(TL_EVENT_INPUT_KEY_PRESSED, on_key_pressed);
 * @endcode
 *
 * **Multiple handlers on same event:**
 * @code
 * // Multiple handlers can subscribe to the same event
 * tl_event_subscribe(TL_EVENT_WINDOW_RESIZED, ui_on_resize);
 * tl_event_subscribe(TL_EVENT_WINDOW_RESIZED, render_on_resize);
 * tl_event_subscribe(TL_EVENT_WINDOW_RESIZED, audio_on_resize);  // May be no-op
 *
 * // All handlers called in order, unless one returns CONSUMED
 * @endcode
 *
 * @note Window and input events are automatically submitted by GLFW callbacks.
 *       Application code only needs to subscribe and handle them.
 *
 * @note Handler order: Handlers are called in subscription order.
 *       First handler to return CONSUMED stops propagation.
 *
 * @note Event data is only valid during the handler call. Do not store
 *       pointers to the event data; copy what you need.
 *
 * @note Custom events can be defined by adding constants after TL_EVENT_MAXIMUM.
 *       Maximum event code is limited by u16 range (65535).
 *
 * @see window.h - Window query and management
 * @see platform.h - Platform layer that submits window events
 *
 * @author TELEIOS Team
 * @version 0.1.0
 */

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
