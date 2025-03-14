#ifndef __TELEIOS_STRING__
#define __TELEIOS_STRING__

#include "teleios/defines.h"
#include "teleios/core/memory.h"

TLString* tl_string_create(TLMemoryArena *arena, u64 length);
TLString* tl_string_wrap(TLMemoryArena *arena, const char *string);
TLString* tl_string_slice(TLMemoryArena *arena, TLString *string, u64 offset, u64 length);
TLString* tl_string_view(TLString *string);
const char * tl_string_text(TLString *string);

u64 tl_string_length(TLString *string);
u64 tl_string_index_of(TLString *string, char token);
u64 tl_string_last_index_of(TLString *string, char token);
b8 tl_string_start_with(TLString *string, const char *guess);
b8 tl_string_ends_with(TLString *string, const char *guess);
b8 tl_string_is_view(TLString *string);
b8 tl_string_equals(TLString *string, const char *guess);
b8 tl_string_contains(TLString *string, const char *guess);

#endif // __TELEIOS_STRING__