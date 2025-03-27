#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

b8 tl_input_is_key_active(const i32 key) {
    TLSTACKPUSHA("%d", key)
    const b8 result = global->platform.input.keyboard.key[key];
    TLSTACKPOPV(result)
}

b8 tl_input_is_key_pressed(const i32 key) {
    TLSTACKPUSHA("%d", key)
    const b8 frame_previous = global->application.frame.last.input.keyboard.key[key];
    const b8 frame_current = global->application.frame.current.input.keyboard.key[key];
    TLSTACKPOPV(!frame_previous && frame_current)
}

b8 tl_input_is_key_released(const i32 key) {
    TLSTACKPUSHA("%d", key)
    const b8 frame_previous = global->application.frame.last.input.keyboard.key[key];
    const b8 frame_current = global->application.frame.current.input.keyboard.key[key];
    TLSTACKPOPV(frame_previous && !frame_current)
}