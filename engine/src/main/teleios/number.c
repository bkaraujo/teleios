#include "teleios/teleios.h"

u8 tl_number_i32_digits(const i32 number) {
    TL_PROFILER_PUSH_WITH("%d", number)

    if (number == 0) {
        TL_PROFILER_POP_WITH(1)
    }

    u8 digits = 0;
    i32 desired = number;

    // Handle negative numbers - convert to positive for counting
    if (desired < 0) {
        digits++; // Count the minus sign
        // Handle INT32_MIN special case (can't be negated without overflow)
        if (desired == (i32)(-2147483647 - 1)) {
            desired = 2147483647; // One less than abs value
            digits++; // Will count one extra digit
        } else {
            desired = -desired; // Make positive
        }
    }

    // Count digits
    while (desired > 0) {
        digits++;
        desired = desired / 10;
    }

    TL_PROFILER_POP_WITH(digits)
}

TLString* tl_number_i32_to_char(TLAllocator* allocator, i32 value, const u8 base){
    TL_PROFILER_PUSH_WITH("0x%p, %d, %d", allocator, value, base)

    // Check that the base is valid
    if (base < 2 || base > 36) {
        TLWARN("Attempting to convert invalid base");
        TL_PROFILER_POP_WITH(NULL)
    }

    // Buffer for conversion: 33 bytes handles binary (-2147483648 = 32 bits + sign + null)
    char buffer[33];
    char *ptr = buffer;
    b8 is_negative = 0;

    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        // Handle INT32_MIN special case to avoid overflow
        if (value == (i32)(-2147483647 - 1)) {
            // Convert manually to avoid overflow when negating
            u32 unsigned_value = 2147483648U;
            do {
                const u32 remainder = unsigned_value % base;
                *ptr++ = (char)((remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10));
                unsigned_value /= base;
            } while (unsigned_value > 0);
        } else {
            value = -value; // Make positive
            u32 unsigned_value = (u32)value;
            do {
                const u32 remainder = unsigned_value % base;
                *ptr++ = (char)((remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10));
                unsigned_value /= base;
            } while (unsigned_value > 0);
        }
    } else {
        // Positive number or zero
        u32 unsigned_value = (u32)value;
        do {
            const u32 remainder = unsigned_value % base;
            *ptr++ = (char)((remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10));
            unsigned_value /= base;
        } while (unsigned_value > 0);
    }

    // Add negative sign if needed
    if (is_negative) {
        *ptr++ = '-';
    }

    // Null terminate
    *ptr = '\0';

    // Reverse the string
    char *start = buffer;
    char *end = ptr - 1;
    while (start < end) {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }

    TLString* string = tl_string_create(allocator, buffer);
    TL_PROFILER_POP_WITH(string)
}

u32 tl_number_next_power_of_2(u32 n) {
    TL_PROFILER_PUSH_WITH("%u", n)

    if (n == 0) {
        TL_PROFILER_POP_WITH(1)
    }

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    TL_PROFILER_POP_WITH(n)
}