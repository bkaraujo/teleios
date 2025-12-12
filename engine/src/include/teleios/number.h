#ifndef __TELEIOS_NUMBER__
#define __TELEIOS_NUMBER__

#include "teleios/defines.h"

// Digit counting
u8 tl_number_signed_digits(i64 number);
u8 tl_number_unsigned_digits(u64 number);

// Number to string conversion
TLString* tl_number_i8_to_char(TLAllocator* allocator, i8 value, u8 base);
TLString* tl_number_i16_to_char(TLAllocator* allocator, i16 value, u8 base);
TLString* tl_number_i32_to_char(TLAllocator* allocator, i32 value, u8 base);
TLString* tl_number_i64_to_char(TLAllocator* allocator, i64 value, u8 base);

TLString* tl_number_u8_to_char(TLAllocator* allocator, u8 value, u8 base);
TLString* tl_number_u16_to_char(TLAllocator* allocator, u16 value, u8 base);
TLString* tl_number_u32_to_char(TLAllocator* allocator, u32 value, u8 base);
TLString* tl_number_u64_to_char(TLAllocator* allocator, u64 value, u8 base);

// Utility
u32 tl_number_next_power_of_2(u32 n);

// String to number conversion
u8 tl_number_u8_from_string(TLString* str);
u16 tl_number_u16_from_string(TLString* str);
u32 tl_number_u32_from_string(TLString* str);
u64 tl_number_u64_from_string(TLString* str);

i8 tl_number_i8_from_string(TLString* str);
i16 tl_number_i16_from_string(TLString* str);
i32 tl_number_i32_from_string(TLString* str);
i64 tl_number_i64_from_string(TLString* str);

f32 tl_number_f32_from_string(TLString* str);
f64 tl_number_f64_from_string(TLString* str);

vec4s tl_number_vec4s_from_string(const TLString* str);

#endif
