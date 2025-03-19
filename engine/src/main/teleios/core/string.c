#include "teleios/core.h"
#include <string.h>

TLINLINE u32 tl_char_length(const char *string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL ) return -1;
    if (*string == '\0') return 0;

    u32 index = 0;

    const char* character = string;
    while (*character++ != '\0') {
        if (index++ == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TLSTACKPOP
    return index;
}

TLINLINE u32 tl_char_last_index(const char *string, const char token) {
    TLSTACKPUSHA("0x%p, %c", string, token)
    u32 index = 0;
    const char* s = string;
    for (u16 i = 0; *s != '\0' ; ++s) {
        if (*s == token) {
            index = i + 1;
        }

        i++;
        if (i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TLSTACKPOP
    return index;
}

b8 tl_char_equals(const char *string, const char *guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOP
        return FALSE;
    }

    const u64 length = tl_char_length(string);
    if (tl_char_length(guess) != length) {
        TLSTACKPOP
        return FALSE;
    }

    for (u64 i = 0; i < length; i++) {
        if (string[i] != guess[i]) {
            TLSTACKPOP
            return FALSE;
        }
    }

    TLSTACKPOP
    return TRUE;
}

u32 tl_char_copy(char *target, const char *source) {
    TLSTACKPUSHA("0x%p, 0x%p", target, source)
    const u32 target_length = tl_char_length(target);
    if (target_length == 0 || target_length == U32_MAX) {
        TLSTACKPOP
        return 0;
    }

    const u32 source_length = tl_char_length(source);
    if (source_length == 0 || source_length == U32_MAX) {
        TLSTACKPOP
        return 0;
    }

    const char *s = source;
    char *t = target;

    u32 copied = 0;
    while (*s != '\0' && *t != '\0') {
        *t = *s;
        s++; t++;
        copied++;
    }

    TLSTACKPOP
    return copied;
}

struct TLString {
    u64 length;
    const char *text;
    TLMemoryArena *arena;
    b8 is_view;
};

TLString* tl_string_wrap(TLMemoryArena *arena, const char *string) {
    TLSTACKPUSHA("0x%p, 0x%p", arena, string)
    TLString *wrap = tl_memory_alloc(arena, sizeof(struct TLString), TL_MEMORY_STRING);
    wrap->arena = arena;
    wrap->is_view = FALSE;
    wrap->length = tl_char_length(string);
    wrap->text = tl_memory_alloc(arena, wrap->length, TL_MEMORY_STRING);
    tl_memory_copy((void*)wrap->text, (void*)string, wrap->length);

    TLSTACKPOP
    return wrap;
}

const char * tl_string_text(TLString *string) {
    TLSTACKPUSHA("0x%p", string)
    const char *text = string->text;
    TLSTACKPOP
    return text;
}

TLString* tl_string_slice(TLMemoryArena *arena, TLString* string, const u64 offset, const u64 length) {
    TLSTACKPUSHA("0x%p, 0x%p, %d, %d", arena, string, offset, length)
    //TODO implement tl_string_slice
    TLFATAL("Implementation missing")
    TLSTACKPOP
    return NULL;
}

TLString* tl_string_view(TLString* string) {
    TLSTACKPUSHA("0x%p", string)
    //TODO implement tl_string_view
    TLFATAL("Implementation missing")
    TLSTACKPOP
    return NULL;
}

u32 tl_string_length(TLString *string) {
    TLSTACKPUSHA("0x%p", string)
    //TODO implement tl_string_length
    TLFATAL("Implementation missing")
    TLSTACKPOP
    return FALSE;
}

u32 tl_string_index_of(TLString* string, char token) {
    TLSTACKPUSHA("0x%p, %s", string, token)
    //TODO implement tl_string_index_of
    TLFATAL("Implementation missing")
    TLSTACKPOP
    return FALSE;
}

u32 tl_string_last_index_of(TLString* string, char token) {
    TLSTACKPUSHA("0x%p, %s", string, token)
    //TODO implement tl_string_last_index_of
    TLFATAL("Implementation missing")
    TLSTACKPOP
    return FALSE;
}

b8 tl_string_start_with(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_start_with
    TLSTACKPOP
    return FALSE;
}

b8 tl_string_ends_with(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_ends_with
    TLFATAL("Implementation missing")
    TLSTACKPOP
    return FALSE;
}

b8 tl_string_is_view(const TLString* string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL) {
        TLSTACKPOP
        return FALSE;
    }

    const b8 is_view = string->is_view;
    TLSTACKPOP
    return is_view;
}

b8 tl_string_equals(const TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOP
        return FALSE;
    }

    const b8 is_equals = tl_char_equals(string->text, guess);
    TLSTACKPOP
    return is_equals;
}

b8 tl_string_contains(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_contains
    TLFATAL("Implementation missing")
    TLSTACKPOP
    return FALSE;
}
