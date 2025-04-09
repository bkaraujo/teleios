#include "teleios/core.h"

K_INLINE u16 tl_number_i32_digits(const i32 number) {
    K_FRAME_PUSH_WITH("%d", number)
    u16 digits = 0;
    i32 desired = number;

    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);
    K_FRAME_POP_WITH(digits)
}