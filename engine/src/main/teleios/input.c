#include "teleios/teleios.h"
#include <GLFW/glfw3.h>

typedef struct {
    struct {
        b8 key[GLFW_KEY_LAST + 1];
    } keyboard;
    struct {
        b8 button[GLFW_MOUSE_BUTTON_LAST + 1];
        f32 position_x;
        f32 position_y;
        i8 scroll_x;
        i8 scroll_y;
        b8 hoover;
    } cursor;
} TLInput;

/** @brief Current frame input state (written by main thread, protected by mutex) */
static TLInput m_current = { 0 };

/** @brief Previous frame snapshot (for pressed/released detection) */
static TLInput m_previous = { 0 };

void tl_input_update() {
    tl_memory_copy( &m_previous, &m_current, sizeof(TLInput) );
}

// ---------------------------------
// Keyboard Queries
// ---------------------------------

b8 tl_input_is_key_active(const i32 key) {
    return m_current.keyboard.key[key];
}

b8 tl_input_is_key_pressed(const i32 key) {
    return !m_previous.keyboard.key[key] && m_current.keyboard.key[key];
}

b8 tl_input_is_key_released(const i32 key) {
    return m_previous.keyboard.key[key] && !m_current.keyboard.key[key];
}

// ---------------------------------
// Cursor Queries
// ---------------------------------

ivec2s tl_input_get_cursor_scroll() {
    return (ivec2s){ m_current.cursor.scroll_x, m_current.cursor.scroll_y };
}

vec2s tl_input_get_cursor_position() {
    return (vec2s){ m_current.cursor.position_x, m_current.cursor.position_y };
}

b8 tl_input_is_cursor_hovering() {
    return m_current.cursor.hoover;
}

b8 tl_input_is_cursor_button_active(const i32 key) {
    return m_current.cursor.button[key];
}

b8 tl_input_is_cursor_button_pressed(const i32 key) {
    return !m_previous.cursor.button[key] && m_current.cursor.button[key];
}

b8 tl_input_is_cursor_button_released(const i32 key) {
    return m_previous.cursor.button[key] && !m_current.cursor.button[key];
}

// ---------------------------------
// Input Lifecycle
// ---------------------------------

static TLEventStatus tl_input_handle_keyboard_pressed(const TLEvent *event) {
    m_current.keyboard.key[event->i32[0]] = true;
    return TL_EVENT_AVAILABLE;
}

static TLEventStatus tl_input_handle_keyboard_released(const TLEvent *event) {
    m_current.keyboard.key[event->i32[0]] = false;
    return TL_EVENT_AVAILABLE;
}

static TLEventStatus tl_input_handle_cursor_moved(const TLEvent *event) {
    m_current.cursor.position_x = event->f32[0];
    m_current.cursor.position_y = event->f32[1];
    return TL_EVENT_AVAILABLE;
}

static TLEventStatus tl_input_handle_cursor_pressed(const TLEvent *event) {
    m_current.cursor.button[event->i32[0]] = true;
    return TL_EVENT_AVAILABLE;
}

static TLEventStatus tl_input_handle_cursor_released(const TLEvent *event) {
    m_current.cursor.button[event->i32[0]] = false;
    return TL_EVENT_AVAILABLE;
}

static TLEventStatus tl_input_handle_cursor_scrolled(const TLEvent *event) {
    m_current.cursor.scroll_x = event->i8[0];
    m_current.cursor.scroll_y = event->i8[1];
    return TL_EVENT_AVAILABLE;
}

static TLEventStatus tl_input_handle_cursor_entered(const TLEvent *event) {
    (void)event;
    m_current.cursor.hoover = true;
    return TL_EVENT_AVAILABLE;
}

static TLEventStatus tl_input_handle_cursor_exited(const TLEvent *event) {
    (void)event;
    m_current.cursor.hoover = false;
    return TL_EVENT_AVAILABLE;
}

b8 tl_input_initialize(void) {
    tl_event_subscribe(TL_EVENT_INPUT_KEY_PRESSED       , tl_input_handle_keyboard_pressed);
    tl_event_subscribe(TL_EVENT_INPUT_KEY_RELEASED      , tl_input_handle_keyboard_released);

    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_MOVED      , tl_input_handle_cursor_moved);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_PRESSED    , tl_input_handle_cursor_pressed);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_RELEASED   , tl_input_handle_cursor_released);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_SCROLLED   , tl_input_handle_cursor_scrolled);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_ENTERED    , tl_input_handle_cursor_entered);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_EXITED     , tl_input_handle_cursor_exited);

    return true;
}