#ifndef __TELEIOS_NUMBER_UNSIGNED__
#define __TELEIOS_NUMBER_UNSIGNED__

#include "teleios/teleios.h"

u8 tl_number_unsigned_digits(const u64 number) {
    TL_PROFILER_PUSH_WITH("%llu", number)

    if (number == 0) {
        TL_PROFILER_POP_WITH(1)
    }

    u8 digits = 0;
    u64 desired = number;

    // Count digits
    while (desired > 0) {
        digits++;
        desired = desired / 10;
    }

    TL_PROFILER_POP_WITH(digits)
}

TLString* tl_number_unsigned_to_char(TLAllocator* allocator, u64 value, const u8 base) {
    TL_PROFILER_PUSH_WITH("0x%p, %llu, %u", allocator, value, base)

    // Check that the base is valid
    if (base < 2 || base > 36) {
        TLERROR("Attempted to  convert invalid base");
        TL_PROFILER_POP_WITH(NULL)
    }

    // Buffer for conversion: 65 bytes handles binary (64 bits + null)
    char buffer[65];
    char *ptr = buffer;

    // Convert to string (reverse order)
    do {
        const u64 remainder = value % base;
        *ptr++ = (char)((remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10));
        value /= base;
    } while (value > 0);

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

TLString* tl_number_u8_to_char(TLAllocator* allocator, u8 value, const u8 base) {
    return tl_number_unsigned_to_char(allocator, (u64)value, base);
}

TLString* tl_number_u16_to_char(TLAllocator* allocator, u16 value, const u8 base) {
    return tl_number_unsigned_to_char(allocator, (u64)value, base);
}

TLString* tl_number_u32_to_char(TLAllocator* allocator, u32 value, const u8 base) {
    return tl_number_unsigned_to_char(allocator, (u64)value, base);
}

TLString* tl_number_u64_to_char(TLAllocator* allocator, u64 value, const u8 base) {
    return tl_number_unsigned_to_char(allocator, value, base);
}

#endif