#ifndef __TELEIOS_STRING__
#define __TELEIOS_STRING__

#include "teleios/defines.h"

typedef struct TLString TLString;

TLString* tl_string_create(u64 length);
void tl_string_destroy(TLString* string);

TLString* tl_string_view(TLString* string);
TLString* tl_string_start_with(TLString* string, const char* guess);
TLString* tl_string_ends_with(TLString* string, const char* guess);
TLString* tl_string_slice(TLString* string, u64 offset);

u64 tl_string_length(TLString *string);
b8 tl_string_is_view(TLString* string);
b8 tl_string_equals(TLString* string, const char* guess);
b8 tl_string_contains(TLString* string, const char* guess);
u64 tl_string_index_of(TLString* string, char token);
u64 tl_string_last_index_of(TLString* string, char token);

#endif // __TELEIOS_STRING__