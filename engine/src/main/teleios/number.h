#ifndef __TELEIOS_NUMBER__
#define __TELEIOS_NUMBER__

#include "teleios/defines.h"

u8 tl_number_i32_digits(i32 number);

TLString* tl_number_i32_to_char(TLAllocator* allocator, i32 value, u8 base);

u32 tl_number_next_power_of_2(u32 n);

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

#endif
