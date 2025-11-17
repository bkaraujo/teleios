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

// ---------------------------------
// Helper Functions
// ---------------------------------

static u64 tl_number_unsigned_from_str(TLString* str, const u64 maximum, const char* type_name) {
    TL_PROFILER_PUSH_WITH("0x%p, %llu, %s", str, maximum, type_name)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to %s", type_name);
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const u64 value = strtoull(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    if (value > maximum) {
        TLWARN("Value %llu exceeds %s range (max %llu)", value, type_name, maximum);
        TL_PROFILER_POP_WITH(maximum)
    }

    TL_PROFILER_POP_WITH(value)
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

// ---------------------------------
// Number from String Conversions
// ---------------------------------

u8 tl_number_u8_from_string(TLString* str) {
    return (u8)tl_number_unsigned_from_str(str, UINT8_MAX, "u8");
}

u16 tl_number_u16_from_string(TLString* str) {
    return (u16)tl_number_unsigned_from_str(str, UINT16_MAX, "u16");
}

u32 tl_number_u32_from_string(TLString* str) {
    return (u32)tl_number_unsigned_from_str(str, UINT32_MAX, "u32");
}

u64 tl_number_u64_from_string(TLString* str) {
    return tl_number_unsigned_from_str(str, UINT64_MAX, "u64");
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


// ---------------------------------
// String to Number Conversions (Aliases)
// ---------------------------------

u8 tl_number_string_to_u8(TLString* str) {
    return tl_number_u8_from_string(str);
}

u16 tl_number_string_to_u16(TLString* str) {
    return tl_number_u16_from_string(str);
}

u32 tl_number_string_to_u32(TLString* str) {
    return tl_number_u32_from_string(str);
}

u64 tl_number_string_to_u64(TLString* str) {
    return tl_number_u64_from_string(str);
}

i8 tl_number_string_to_i8(TLString* str) {
    return tl_number_i8_from_string(str);
}

i16 tl_number_string_to_i16(TLString* str) {
    return tl_number_i16_from_string(str);
}

i32 tl_number_string_to_i32(TLString* str) {
    return tl_number_i32_from_string(str);
}

i64 tl_number_string_to_i64(TLString* str) {
    return tl_number_i64_from_string(str);
}

f32 tl_number_string_to_f32(const TLString* str) {
    return tl_number_f32_from_string((TLString*)str);
}

f64 tl_number_string_to_f64(const TLString* str) {
    return tl_number_f64_from_string((TLString*)str);
}
