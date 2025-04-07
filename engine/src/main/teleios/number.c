#include "teleios/core.h"

BKS_INLINE u16 tl_number_i32_digits(const i32 number) {
    BKS_STACK_PUSHA("%d", number)
    u16 digits = 0;
    i32 desired = number;

    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);
    BKS_STACK_POPV(digits)
}