#include "teleios/teleios.h"

u8 tl_number_i32_digits(const i32 number) {
    TL_PROFILER_PUSH_WITH("%d", number)

    u8 digits = 0;
    i32 desired = number;

    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);

    TL_PROFILER_POP_WITH(digits)
}

void tl_number_i32_to_char(const char* buffer, i32 value, u8 base){
    TL_PROFILER_PUSH_WITH("0x%p, 0x%p, %d, %d", buffer, value, base)

    // check that the base if valid
    if (base < 2 || base > 36) { TL_PROFILER_POP }

    int tmp_value;
    char *ptr  = (char*) buffer;
    char *ptr1 = (char*) buffer;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';

    // Reverse the string
    while(ptr1 < ptr) {
        const char tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    TL_PROFILER_POP
}