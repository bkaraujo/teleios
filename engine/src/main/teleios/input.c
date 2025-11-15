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

/** @brief Current frame input state*/
static TLInput current = { 0 };

/** @brief Previous frame input state*/
static TLInput previous = { 0 };

void tl_input_update() {
    TL_PROFILER_PUSH
    tl_memory_copy( &previous, &current, sizeof(TLInput) );
    TL_PROFILER_POP
}

// ---------------------------------
// Keyboard Queries
// ---------------------------------

b8 tl_input_is_key_active(const i32 key) {
    TL_PROFILER_PUSH_WITH("%d", key)
    const b8 is_active = current.keyboard.key[key];
    TL_PROFILER_POP_WITH(is_active)
}

b8 tl_input_is_key_pressed(const i32 key) {
    TL_PROFILER_PUSH_WITH("%d", key)
    const b8 is_active = current.keyboard.key[key];
    const b8 were_active = previous.keyboard.key[key];
    TL_PROFILER_POP_WITH(!were_active & is_active)
}

b8 tl_input_is_key_released(const i32 key) {
    TL_PROFILER_PUSH_WITH("%d", key)
    const b8 is_active = current.keyboard.key[key];
    const b8 were_active = previous.keyboard.key[key];
    TL_PROFILER_POP_WITH(were_active & !is_active)
}

// ---------------------------------
// Cursor Queries
// ---------------------------------

ivec2s tl_input_get_cursor_scroll() {
    TL_PROFILER_PUSH
    const ivec2s scroll = { current.cursor.scroll_x, current.cursor.scroll_y };
    TL_PROFILER_POP_WITH(scroll)
}

vec2s tl_input_get_cursor_position() {
    TL_PROFILER_PUSH
    const vec2s position = { current.cursor.position_x, current.cursor.position_y };
    TL_PROFILER_POP_WITH(position)
}

b8 tl_input_is_cursor_hovering() {
    TL_PROFILER_PUSH
    const b8 is_active = current.cursor.hoover;
    TL_PROFILER_POP_WITH(is_active)
}

b8 tl_input_is_cursor_button_active(const i32 key) {
    TL_PROFILER_PUSH_WITH("%d", key)
    const b8 is_active = current.cursor.button[key];
    TL_PROFILER_POP_WITH(is_active)
}

b8 tl_input_is_cursor_button_pressed(const i32 key) {
    TL_PROFILER_PUSH_WITH("%d", key)
    const b8 is_active = current.cursor.button[key];
    const b8 were_active = previous.cursor.button[key];
    TL_PROFILER_POP_WITH(!were_active & is_active)
}

b8 tl_input_is_cursor_button_released(const i32 key) {
    TL_PROFILER_PUSH_WITH("%d", key)
    const b8 is_active = current.cursor.button[key];
    const b8 were_active = previous.cursor.button[key];
    TL_PROFILER_POP_WITH(were_active & !is_active)
}

// ---------------------------------
// Input Lifecycle
// ---------------------------------

static TLEventStatus tl_input_handle_keyboard_pressed(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.keyboard.key[event->i32[0]] = true;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_input_handle_keyboard_released(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.keyboard.key[event->i32[0]] = false;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_input_handle_cursor_moved(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.cursor.position_x = event->f32[0];
    current.cursor.position_y = event->f32[1];
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_input_handle_cursor_pressed(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.cursor.button[event->i32[0]] = true;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_input_handle_cursor_released(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.cursor.button[event->i32[0]] = false;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_input_handle_cursor_scrolled(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.cursor.scroll_x = event->i8[0];
    current.cursor.scroll_y = event->i8[1];
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_input_handle_cursor_entered(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.cursor.hoover = true;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

static TLEventStatus tl_input_handle_cursor_exited(const TLEvent *event) {
    TL_PROFILER_PUSH_WITH("0x%p", event)
    current.cursor.hoover = false;
    TL_PROFILER_POP_WITH(TL_EVENT_AVAILABLE)
}

b8 tl_input_initialize(void) {
    TL_PROFILER_PUSH

    tl_event_subscribe(TL_EVENT_INPUT_KEY_PRESSED       , tl_input_handle_keyboard_pressed);
    tl_event_subscribe(TL_EVENT_INPUT_KEY_RELEASED      , tl_input_handle_keyboard_released);

    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_MOVED      , tl_input_handle_cursor_moved);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_PRESSED    , tl_input_handle_cursor_pressed);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_RELEASED   , tl_input_handle_cursor_released);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_SCROLLED   , tl_input_handle_cursor_scrolled);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_ENTERED    , tl_input_handle_cursor_entered);
    tl_event_subscribe(TL_EVENT_INPUT_CURSOR_EXITED     , tl_input_handle_cursor_exited);

    TL_PROFILER_POP_WITH(true)
}