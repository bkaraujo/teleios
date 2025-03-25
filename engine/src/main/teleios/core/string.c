#include "teleios/core.h"
#include <string.h>

TLINLINE u32 tl_char_length(const char *string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL ) TLSTACKPOPV(U32_MAX);
    if (*string == '\0') TLSTACKPOPV(0);

    u32 index = 0;

    const char* character = string;
    while (*character++ != '\0') {
        if (index++ == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TLSTACKPOPV(index)
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

    TLSTACKPOPV(index)
}

TLINLINE u32 tl_char_index_of(const char *string, const char token) {
    TLSTACKPUSHA("0x%p, %c", string, token)
    const char* s = string;
    for (u16 i = 0; *s != '\0' ; ++s) {
        if (*s == token) {
            TLSTACKPOPV(i)
        }

        i++;
        if (i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TLSTACKPOPV(U32_MAX)
}

b8 tl_char_equals(const char *string, const char *guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOPV(FALSE)
    }

    const u64 length = tl_char_length(string);
    if (tl_char_length(guess) != length) {
        TLSTACKPOPV(FALSE)
    }

    for (u64 i = 0; i < length; i++) {
        if (string[i] != guess[i]) {
            TLSTACKPOPV(FALSE)
        }
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_char_start_with(const char *string, const char *guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOPV(FALSE)
    }

    const u64 length = tl_char_length(guess);
    if (length > tl_char_length(string)) {
        TLSTACKPOPV(FALSE)
    }

    for (u64 i = 0; i < length; i++) {
        if (string[i] != guess[i]) {
            TLSTACKPOPV(FALSE)
        }
    }

    TLSTACKPOPV(TRUE)
}

u32 tl_char_copy(char *target, const char *source) {
    TLSTACKPUSHA("0x%p, 0x%p", target, source)
    const u32 target_length = tl_char_length(target);
    if (target_length == 0 || target_length == U32_MAX) {
        TLSTACKPOPV(0)
    }

    const u32 source_length = tl_char_length(source);
    if (source_length == 0 || source_length == U32_MAX) {
        TLSTACKPOPV(0)
    }

    const char *s = source;
    char *t = target;

    u32 copied = 0;
    while (*s != '\0' && *t != '\0') {
        *t = *s;
        s++; t++;
        copied++;
    }

    TLSTACKPOPV(copied)
}

TLINLINE void tl_char_join(char *buffer, const u64 size, const char *str0, const char *str1) {
    TLSTACKPUSHA("0x%p, %d, %s, %s", buffer, size, str0, str1)
    if (tl_char_length(str0) + tl_char_length(str1) > size) TLSTACKPOP
    sprintf(buffer, "%s%s", str0, str1);
    TLSTACKPOP
}

struct TLString {
    u64 length;
    u64 size;
    const char *text;
    TLMemoryArena *arena;
    b8 is_view;
};

TLINLINE static TLString* tl_string_reserve(TLMemoryArena *arena, const u64 size) {
    TLSTACKPUSHA("0x%p, %d", arena, size)
    TLString *string = tl_memory_alloc(arena, sizeof(struct TLString), TL_MEMORY_STRING);
    string->is_view = FALSE;
    string->arena = arena;
    string->size = size;
    string->length = 0;
    if (string->size > 0)
        string->text = tl_memory_alloc(arena, string->size, TL_MEMORY_STRING);

    TLSTACKPOPV(string)
}

TLString* tl_string_clone(TLMemoryArena *arena, const char *string) {
    TLSTACKPUSHA("0x%p, 0x%p", arena, string)
    TLString *clone = tl_string_reserve(arena, tl_char_length(string));
    clone->length = clone->size;
    tl_memory_copy((void*)clone->text, (void*)string, clone->size);
    TLSTACKPOPV(clone)
}

TLString* tl_string_wrap(TLMemoryArena *arena, const char *string) {
    TLSTACKPUSHA("0x%p, 0x%p", arena, string)
    TLString *wrap = tl_string_reserve(arena, 0);
    wrap->is_view = TRUE;
    wrap->text = string;
    wrap->size = wrap->length;
    wrap->length = tl_char_length(string);
    TLSTACKPOPV(wrap)
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
TLString* tl_string_from_i32(TLMemoryArena *arena, i32 value, u8 base) {
    TLSTACKPUSHA("%d, %d", value, base)

    u32 digits = 0;
    i32 desired = value;
    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);

    TLString* string = tl_string_reserve(arena, digits);

    // check that the base if valid
    if (base < 2 || base > 36) { TLSTACKPOPV(string) }

    int tmp_value;
    char *ptr  = (char*) string->text;
    char *ptr1 = (char*) string->text;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
        string->length++;
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';

    // Reverse the string
    while(ptr1 < ptr) {
        const char tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
        string->length++;
    }

    TLSTACKPOPV(string)
}

void tl_string_join(const TLString *string, const char *other) {
    TLSTACKPUSHA("0x%p, 0x%p", string, other)
    const u64 length = tl_char_length(other);
    if (length == U32_MAX || length == 0) TLFATAL("Failed to join:\n\n - 0%xs\n - %s", string->text, other)
    const u64 created_length = string->length + length;
    void *created = tl_memory_alloc(string->arena, created_length, TL_MEMORY_STRING);
    tl_memory_copy(created, (void*)string->text, string->length);
    tl_memory_copy(created + string->length, (void*)other, length);

    ((TLString*)string)->text = created;
    ((TLString*)string)->size = created_length;
    ((TLString*)string)->length = created_length;
    ((TLString*)string)->is_view = FALSE;

    TLSTACKPOP
}

TLINLINE const char * tl_string(TLString *string) {
    TLSTACKPUSHA("0x%p", string)
    TLSTACKPOPV(string->text)
}

TLString* tl_string_slice(TLMemoryArena *arena, TLString* string, const u64 offset, const u64 length) {
    TLSTACKPUSHA("0x%p, 0x%p, %d, %d", arena, string, offset, length)
    //TODO implement tl_string_slice
    TLFATAL("Implementation missing")
    TLSTACKPOPV(NULL)
}

TLString* tl_string_view(TLString* string) {
    TLSTACKPUSHA("0x%p", string)
    TLString *view = tl_string_reserve(string->arena, 0);
    view->is_view = TRUE;
    view->text = string->text;
    view->size = string->size;
    view->length = string->length;
    TLSTACKPOPV(view)
}

u32 tl_string_length(TLString *string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL) {
        TLSTACKPOPV(-1)
    }
    TLSTACKPOPV(string->length)
}

u32 tl_string_index_of(TLString* string, const char token) {
    TLSTACKPUSHA("0x%p, %s", string, token)
    for (u64 i = 0; i < string->length; i++) {
        if (string->text[i] == token) TLSTACKPOPV(i);
    }
    TLSTACKPOPV(U32_MAX)
}

u32 tl_string_last_index_of(TLString* string, const char token) {
    TLSTACKPUSHA("0x%p, %s", string, token)
    if (string == NULL) TLSTACKPOPV(U32_MAX);

    if (string->length == 0) {
        if (string->text[0] == token) TLSTACKPOPV(0);
        TLSTACKPOPV(U32_MAX)
    }

    for (u64 i = string->length - 1; i > 0; --i) {
        if (string->text[i] == token) TLSTACKPOPV(i);
    }
    TLFATAL("Implementation missing")
    TLSTACKPOPV(FALSE)
}

b8 tl_string_start_with(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) TLSTACKPOPV(FALSE)

    const u64 length = tl_char_length(guess);
    if (length > string->length) TLSTACKPOPV(FALSE)

    for (u64 i = 0; i < length; ++i) {
        if (string->text[i] != guess[i])
            TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_string_ends_with(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_ends_with
    TLFATAL("Implementation missing")
    TLSTACKPOPV(FALSE)
}

b8 tl_string_is_view(const TLString* string) {
    TLSTACKPUSHA("0x%p", string)
    if (string == NULL) {
        TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(string->is_view)
}

b8 tl_string_equals(const TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TLSTACKPOPV(FALSE)
    }

    const u64 length = tl_char_length(guess);
    if (string->length != length) TLSTACKPOPV(FALSE)

    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] != guess[i])
            TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(TRUE)
}

b8 tl_string_contains(TLString* string, const char* guess) {
    TLSTACKPUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_contains
    TLFATAL("Implementation missing")
    TLSTACKPOPV(FALSE)
}