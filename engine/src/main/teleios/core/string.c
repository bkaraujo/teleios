#include "teleios/core.h"

struct TLString {
    u64 length;
    const char *text;
    TLMemoryArena *arena;
    b8 is_view;
};

TLString* tl_string_create(TLMemoryArena *arena, u64 length) {
    return NULL;
}

TLString* tl_string_wrap(TLMemoryArena *arena, const char *string) {
    TLTRACE(">> tl_string_wrap(0x%p, 0x%p)", arena, string)
    TLString *wrap = tl_memory_alloc(arena, sizeof(struct TLString), TL_MEMORY_STRING);
    wrap->arena = arena;
    wrap->is_view = FALSE;
    wrap->length = __builtin_strlen(string);
    wrap->text = tl_memory_alloc(arena, wrap->length, TL_MEMORY_STRING);
    tl_memory_copy((void*)wrap->text, (void*)string, wrap->length);

    TLTRACE("<< tl_string_wrap(0x%p, 0x%p)", arena, string)
    return wrap;
}

const char * tl_string_text(TLString *string) {
    TLTRACE("<< tl_string_text(0x%p)", string)
    TLVERBOSE("Reading string from arena 0x%p", string->arena);
    TLTRACE("<< tl_string_text(0x%p)", string)
    return string->text;
}

TLString* tl_string_slice(TLMemoryArena *arena, TLString* string, u64 offset, u64 length) {
    return NULL;
}

TLString* tl_string_view(TLString* string) {
    return NULL;
}

u64 tl_string_length(TLString *string) {
    return FALSE;
}

u64 tl_string_index_of(TLString* string, char token) {
    return FALSE;
}

u64 tl_string_last_index_of(TLString* string, char token) {
    return FALSE;
}

b8 tl_string_start_with(TLString* string, const char* guess) {
    return FALSE;
}

b8 tl_string_ends_with(TLString* string, const char* guess) {
    return FALSE;
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
