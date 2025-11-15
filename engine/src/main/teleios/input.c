#include "teleios/teleios.h"
#include <GLFW/glfw3.h>

typedef struct {
    struct {
        b8 key[GLFW_KEY_LAST + 1];
    } keyboard;
    struct {
        b8 button[GLFW_MOUSE_BUTTON_LAST + 1];
        i32 position_x;
        i32 position_y;
        u8 scroll_x;
        u8 scroll_y;
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

ivec2s tl_input_get_cursor_scroll() {
    TL_PROFILER_PUSH
    const ivec2s scroll = { current.cursor.scroll_x, current.cursor.scroll_y };
    TL_PROFILER_POP_WITH(scroll)
}

ivec2s tl_input_get_cursor_position() {
    TL_PROFILER_PUSH
    const ivec2s position = { current.cursor.position_x, current.cursor.position_y };
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
