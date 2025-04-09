#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

void tl_input_update() {
    k_memory_copy(
        &global->application.frame.last.input,
        &global->application.frame.current.input,
        sizeof(TLInput)
    );

    k_memory_copy(
        &global->application.frame.current.input,
        &global->platform.input,
        sizeof(TLInput)
    );
}

b8 tl_input_is_key_active(const i32 key) {
    K_FRAME_PUSH_WITH("%d", key)
    const b8 is_active = global->application.frame.current.input.keyboard.key[key];
    K_FRAME_POP_WITH(is_active)
}

b8 tl_input_is_key_pressed(const i32 key) {
    K_FRAME_PUSH_WITH("%d", key)
    const b8 is_active = global->application.frame.current.input.keyboard.key[key];
    const b8 were_active = global->application.frame.last.input.keyboard.key[key];
    K_FRAME_POP_WITH(!were_active & is_active)
}

b8 tl_input_is_key_released(const i32 key) {
    K_FRAME_PUSH_WITH("%d", key)
    const b8 is_active = global->application.frame.current.input.keyboard.key[key];
    const b8 were_active = global->application.frame.last.input.keyboard.key[key];
    K_FRAME_POP_WITH(were_active & !is_active)
}