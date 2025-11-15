#ifndef __TELEIOS_INPUT__
#define __TELEIOS_INPUT__

#include "teleios/defines.h"

b8 tl_input_initialize(void);
void tl_input_update(void);

b8 tl_input_is_key_active(i32 key);
b8 tl_input_is_key_pressed(i32 key);
b8 tl_input_is_key_released(i32 key);

ivec2s tl_input_get_cursor_scroll(void);
vec2s tl_input_get_cursor_position(void);
b8 tl_input_is_cursor_hovering(void);
b8 tl_input_is_cursor_button_active(i32 key);
b8 tl_input_is_cursor_button_pressed(i32 key);
b8 tl_input_is_cursor_button_released(i32 key);

#endif