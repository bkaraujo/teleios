#include "teleios/core.h"
#include <string.h>

TLINLINE u32 tl_char_length(const char *string) {
    if (string == NULL ) return -1;
    if (*string == '\0') return 0;

    u32 index = 0;

    const char* character = string;
    while (*character++ != '\0') {
        if (index++ == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    return index;
}

TLINLINE u32 tl_char_last_index(const char *string, char token) {
    u32 index = 0;
    const char* character = string;
    for (u16 i = 0; *character != '\0' ; ++character) {
        if (*character == token) {
            index = i + 1;
        }

        i++;
        if (i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    return index;
}

b8 tl_char_equals(const char *string, const char *guess) {
    if (string == NULL || guess == NULL) return FALSE;

    const u64 length = tl_char_length(string);
    if (tl_char_length(guess) != length) return FALSE;

    for (u64 i = 0; i < length; i++) {
        if (string[i] != guess[i]) return FALSE;
    }

    return TRUE;
}

struct TLString {
    u64 length;
    const char *text;
    TLMemoryArena *arena;
    b8 is_view;
};

TLString* tl_string_wrap(TLMemoryArena *arena, const char *string) {
    TLTRACE(">> tl_string_wrap(0x%p, 0x%p)", arena, string)
    TLString *wrap = tl_memory_alloc(arena, sizeof(struct TLString), TL_MEMORY_STRING);
    wrap->arena = arena;
    wrap->is_view = FALSE;
    wrap->length = tl_char_length(string);
    wrap->text = tl_memory_alloc(arena, wrap->length, TL_MEMORY_STRING);
    tl_memory_copy((void*)wrap->text, (void*)string, wrap->length);

    TLTRACE("<< tl_string_wrap(0x%p, 0x%p)", arena, string)
    return wrap;
}

const char * tl_string_text(TLString *string) {
    TLTRACE("<< tl_string_text(0x%p)", string)
    TLVERBOSE("Reading string from arena 0x%p", string->arena);
    const char *text = string->text;
    TLTRACE("<< tl_string_text(0x%p)", string)

    return text;
}

TLString* tl_string_slice(TLMemoryArena *arena, TLString* string, u64 offset, u64 length) {
    //TODO implement tl_string_slice
    return NULL;
}

TLString* tl_string_view(TLString* string) {
    //TODO implement tl_string_view
    return NULL;
}

u64 tl_string_length(TLString *string) {
    //TODO implement tl_string_length
    return FALSE;
}

u64 tl_string_index_of(TLString* string, char token) {
    //TODO implement tl_string_index_of
    return FALSE;
}

u64 tl_string_last_index_of(TLString* string, char token) {
    //TODO implement tl_string_last_index_of
    return FALSE;
}

b8 tl_string_start_with(TLString* string, const char* guess) {
    //TODO implement tl_string_start_with
    return FALSE;
}

b8 tl_string_ends_with(TLString* string, const char* guess) {
    //TODO implement tl_string_ends_with
    return FALSE;
}

b8 tl_string_is_view(const TLString* string) {
    if (string == NULL) return FALSE;
    return string->is_view;
}

b8 tl_string_equals(const TLString* string, const char* guess) {
    if (string == NULL || guess == NULL) return FALSE;
    return tl_char_equals(string->text, guess);
}

b8 tl_string_contains(TLString* string, const char* guess) {
    //TODO implement tl_string_contains
    return FALSE;
}
