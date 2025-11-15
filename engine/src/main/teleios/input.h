#ifndef __TELEIOS_INPUT__
#define __TELEIOS_INPUT__

#include "teleios/defines.h"

void tl_input_update();

b8 tl_input_is_key_active(i32 key);
b8 tl_input_is_key_pressed(i32 key);
b8 tl_input_is_key_released(i32 key);

ivec2s tl_input_get_cursor_scroll();
ivec2s tl_input_get_cursor_position();
b8 tl_input_is_cursor_hovering();
b8 tl_input_is_cursor_button_active(i32 key);
b8 tl_input_is_cursor_button_pressed(i32 key);
b8 tl_input_is_cursor_button_released(i32 key);

#endif