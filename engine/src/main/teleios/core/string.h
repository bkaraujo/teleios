#ifndef __TELEIOS_CORE_STRING__
#define __TELEIOS_CORE_STRING__

#include "teleios/defines.h"

u32 tl_char_length(const char *string);
u32 tl_char_index_of(const char *string, char token);
u32 tl_char_last_index_of(const char *string, char character);
b8 tl_char_equals(const char *string, const char *guess);
b8 tl_char_start_with(const char *string, const char *guess);
u32 tl_char_copy(char *target, const char *source, u32 length);
void tl_char_join(char *buffer, u64 size, const char *str0, const char *str1);
void tl_char_from_i32(char *buffer, i32 value, u8 base);

#endif // __TELEIOS_CORE_STRING__