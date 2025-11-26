#ifndef __TELEIOS_NUMBER_SIGNED__
#define __TELEIOS_NUMBER_SIGNED__

#include "teleios/teleios.h"

u8 tl_number_signed_digits(const i64 number) {
    TL_PROFILER_PUSH_WITH("%lld", number)

    if (number == 0) {
        TL_PROFILER_POP_WITH(1)
    }

    u8 digits = 0;
    i64 desired = number;

    // Handle negative numbers - convert to positive for counting
    if (desired < 0) {
        digits++; // Count the minus sign
        // Handle INT64_MIN special case (can't be negated without overflow)
        if (desired == I64_MIN) {
            desired = I64_MAX; // Use max value, will add 1 extra digit below
            digits++; // Account for the difference (INT64_MIN = INT64_MAX + 1)
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

TLString* tl_number_signed_to_char(TLAllocator* allocator, i64 value, const u8 base) {
    TL_PROFILER_PUSH_WITH("0x%p, %lld, %u", allocator, value, base)

    // Check that the base is valid
    if (base < 2 || base > 36) {
        TLERROR("Attempted to  convert invalid base");
        TL_PROFILER_POP_WITH(NULL)
    }

    // Buffer for conversion: 66 bytes handles binary (64 bits + sign + null)
    char buffer[66];
    char *ptr = buffer;
    b8 is_negative = 0;

    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        // Handle INT64_MIN special case to avoid overflow
        if (value == I64_MIN) {
            // Convert manually to avoid overflow when negating
            // I64_MIN = -9223372036854775808 = 0x8000000000000000
            u64 unsigned_value = 9223372036854775808ULL;
            do {
                const u64 remainder = unsigned_value % base;
                *ptr++ = (char)((remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10));
                unsigned_value /= base;
            } while (unsigned_value > 0);
        } else {
            value = -value; // Make positive
            u64 unsigned_value = (u64)value;
            do {
                const u64 remainder = unsigned_value % base;
                *ptr++ = (char)((remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10));
                unsigned_value /= base;
            } while (unsigned_value > 0);
        }
    } else {
        // Positive number or zero
        u64 unsigned_value = (u64)value;
        do {
            const u64 remainder = unsigned_value % base;
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
        const char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }

    TLString* string = tl_string_create(allocator, buffer);
    TL_PROFILER_POP_WITH(string)
}

static i64 tl_number_signed_from_str(TLString* str, const i64 minimum, const i64 maximum, const char* type_name) {
    TL_PROFILER_PUSH_WITH("0x%p, %lld, %lld, %s", str, minimum, maximum, type_name)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to %s", type_name);
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const i64 value = strtoll(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    if (value < minimum) {
        TLWARN("Value %lld below %s range (min %lld)", value, type_name, minimum);
        TL_PROFILER_POP_WITH(minimum)
    }

    if (value > maximum) {
        TLWARN("Value %lld exceeds %s range (max %lld)", value, type_name, maximum);
        TL_PROFILER_POP_WITH(maximum)
    }

    TL_PROFILER_POP_WITH(value)
}

i8 tl_number_i8_from_string(TLString* str) {
    return (i8)tl_number_signed_from_str(str, INT8_MIN, INT8_MAX, "i8");
}

i16 tl_number_i16_from_string(TLString* str) {
    return (i16)tl_number_signed_from_str(str, INT16_MIN, INT16_MAX, "i16");
}

i32 tl_number_i32_from_string(TLString* str) {
    return (i32)tl_number_signed_from_str(str, INT32_MIN, INT32_MAX, "i32");
}

i64 tl_number_i64_from_string(TLString* str) {
    return tl_number_signed_from_str(str, INT64_MIN, INT64_MAX, "i64");
}

f32 tl_number_f32_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to f32");
        TL_PROFILER_POP_WITH(0.0f)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const float value = strtof(cstr, &endptr);

    if (endptr == cstr) {
        TLWARN("No valid number found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0.0f)
    }

    TL_PROFILER_POP_WITH(value)
}

f64 tl_number_f64_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to f64");
        TL_PROFILER_POP_WITH(0.0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const double value = strtod(cstr, &endptr);

    if (endptr == cstr) {
        TLWARN("No valid number found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0.0)
    }

    TL_PROFILER_POP_WITH(value)
}

TLString* tl_number_i8_to_char(TLAllocator* allocator, i8 value, const u8 base) {
    return tl_number_signed_to_char(allocator, (i64)value, base);
}

TLString* tl_number_i16_to_char(TLAllocator* allocator, i16 value, const u8 base) {
    return tl_number_signed_to_char(allocator, (i64)value, base);
}

TLString* tl_number_i32_to_char(TLAllocator* allocator, i32 value, const u8 base) {
    return tl_number_signed_to_char(allocator, (i64)value, base);
}

TLString* tl_number_i64_to_char(TLAllocator* allocator, i64 value, const u8 base) {
    return tl_number_signed_to_char(allocator, value, base);
}

#endif