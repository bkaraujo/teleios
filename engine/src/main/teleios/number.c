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
// String to Number Conversions
// ---------------------------------

u8 tl_number_u8_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to u8");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const unsigned long value = strtoul(cstr, &endptr, 10);

    // Check for conversion errors
    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    // Check for overflow
    if (value > UINT8_MAX) {
        TLWARN("Value %lu exceeds u8 range (max %u)", value, UINT8_MAX);
        TL_PROFILER_POP_WITH(UINT8_MAX)
    }

    TL_PROFILER_POP_WITH((u8)value)
}

u16 tl_number_u16_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to u16");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const unsigned long value = strtoul(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    if (value > UINT16_MAX) {
        TLWARN("Value %lu exceeds u16 range (max %u)", value, UINT16_MAX);
        TL_PROFILER_POP_WITH(UINT16_MAX)
    }

    TL_PROFILER_POP_WITH((u16)value)
}

u32 tl_number_u32_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to u32");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const unsigned long long value = strtoull(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    if (value > UINT32_MAX) {
        TLWARN("Value %llu exceeds u32 range (max %u)", value, UINT32_MAX);
        TL_PROFILER_POP_WITH(UINT32_MAX)
    }

    TL_PROFILER_POP_WITH((u32)value)
}

u64 tl_number_u64_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to u64");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const unsigned long long value = strtoull(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    TL_PROFILER_POP_WITH((u64)value)
}

i8 tl_number_i8_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to i8");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const long value = strtol(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    if (value < INT8_MIN) {
        TLWARN("Value %ld below i8 range (min %d)", value, INT8_MIN);
        TL_PROFILER_POP_WITH(INT8_MIN)
    }

    if (value > INT8_MAX) {
        TLWARN("Value %ld exceeds i8 range (max %d)", value, INT8_MAX);
        TL_PROFILER_POP_WITH(INT8_MAX)
    }

    TL_PROFILER_POP_WITH((i8)value)
}

i16 tl_number_i16_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to i16");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const long value = strtol(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    if (value < INT16_MIN) {
        TLWARN("Value %ld below i16 range (min %d)", value, INT16_MIN);
        TL_PROFILER_POP_WITH(INT16_MIN)
    }

    if (value > INT16_MAX) {
        TLWARN("Value %ld exceeds i16 range (max %d)", value, INT16_MAX);
        TL_PROFILER_POP_WITH(INT16_MAX)
    }

    TL_PROFILER_POP_WITH((i16)value)
}

i32 tl_number_i32_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to i32");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const long long value = strtoll(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    if (value < INT32_MIN) {
        TLWARN("Value %lld below i32 range (min %d)", value, INT32_MIN);
        TL_PROFILER_POP_WITH(INT32_MIN)
    }

    if (value > INT32_MAX) {
        TLWARN("Value %lld exceeds i32 range (max %d)", value, INT32_MAX);
        TL_PROFILER_POP_WITH(INT32_MAX)
    }

    TL_PROFILER_POP_WITH((i32)value)
}

i64 tl_number_i64_from_string(TLString* str) {
    TL_PROFILER_PUSH_WITH("0x%p", str)

    if (str == NULL || tl_string_is_empty(str)) {
        TLWARN("Cannot convert NULL or empty string to i64");
        TL_PROFILER_POP_WITH(0)
    }

    const char* cstr = tl_string_cstr(str);
    char* endptr;
    const long long value = strtoll(cstr, &endptr, 10);

    if (endptr == cstr) {
        TLWARN("No valid digits found in string: %s", cstr);
        TL_PROFILER_POP_WITH(0)
    }

    TL_PROFILER_POP_WITH((i64)value)
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