#ifndef __TELEIOS_RUNTIME_STRING__
#define __TELEIOS_RUNTIME_STRING__

#include "teleios/defines.h"

TLString* tl_string_clone(TLMemoryArena *arena, const char *string);
TLString* tl_string_wrap(TLMemoryArena *arena, const char *string);
TLString* tl_string_slice(TLMemoryArena *arena, TLString *string, u64 offset, u64 length);
TLString* tl_string_view(TLString *string);

const char * tl_string(TLString *string);
u32 tl_string_length(TLString *string);
u32 tl_string_index_of(TLString *string, char token);
u32 tl_string_last_index_of(TLString *string, char token);
b8 tl_string_start_with(TLString *string, const char *guess);
b8 tl_string_ends_with(TLString *string, const char *guess);
b8 tl_string_is_view(const TLString *string);
b8 tl_string_equals(const TLString *string, const char *guess);
b8 tl_string_contains(TLString *string, const char *guess);
TLString* tl_string_from_i32(TLMemoryArena *arena, i32 value, u8 base);
void tl_string_join(const TLString *string, const char *other);

#endif // __TELEIOS_RUNTIME_STRING__