#include "teleios/string.h"
#include "teleios/memory.h"

struct TLString {
    u64 length;
    const char* payload;
};

TLString* tl_string_create(u64 length)  {
    return NULL;
}

void tl_string_destroy(TLString* string) {

}

TLString* tl_string_view(TLString* string) {
    return NULL;
}

TLString* tl_string_start_with(TLString* string, const char* guess) {
    return NULL;
}

TLString* tl_string_ends_with(TLString* string, const char* guess) {
    return NULL;
}

TLString* tl_string_slice(TLString* string, u64 offset) {
    return NULL;
}

u64 tl_string_length(TLString *string) {
    return 0;
}

b8 tl_string_is_view(TLString* string) {
    return FALSE;
}

b8 tl_string_equals(TLString* string, const char* guess) {
    return FALSE;
}

b8 tl_string_contains(TLString* string, const char* guess) {
    return FALSE;
}

u64 tl_string_index_of(TLString* string, char token) {
    return 0;
}

u64 tl_string_last_index_of(TLString* string, char token) {
    return 0;
}
