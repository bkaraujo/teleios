#include "teleios/core.h"
#include "teleios/runtime.h"
#include <string.h>
// #####################################################################################################################
//
//                                                     STRING
//
// #####################################################################################################################
TL_INLINE u32 tl_char_length(const char *string) {
    TL_STACK_PUSHA("0x%p", string)
    if (string == NULL ) TL_STACK_POPV(U32_MAX);
    if (*string == '\0') TL_STACK_POPV(0);

    u32 index = 0;

    const char* character = string;
    while (*character++ != '\0') {
        if (index++ == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TL_STACK_POPV(index)
}

TL_INLINE u32 tl_char_last_index_of(const char *string, const char character) {
    TL_STACK_PUSHA("0x%p, %c", string, character)
    u32 index = 0;
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == character) {
            index = i;
        }

        if (++i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TL_STACK_POPV(index)
}

TL_INLINE u32 tl_char_index_of(const char *string, const char token) {
    TL_STACK_PUSHA("0x%p, %c", string, token)
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == token) {
            TL_STACK_POPV(i)
        }

        if (++i == U32_MAX) {
            TLFATAL("Failed to find string length")
        }
    }

    TL_STACK_POPV(U32_MAX)
}

b8 tl_char_equals(const char *string, const char *guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TL_STACK_POPV(false)
    }

    const u64 length = tl_char_length(string);
    if (tl_char_length(guess) != length) {
        TL_STACK_POPV(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            TL_STACK_POPV(false)
        }
    }

    TL_STACK_POPV(true)
}

b8 tl_char_start_with(const char *string, const char *guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TL_STACK_POPV(false)
    }

    const u64 length = tl_char_length(guess);
    if (length > tl_char_length(string)) {
        TL_STACK_POPV(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            TL_STACK_POPV(false)
        }
    }

    TL_STACK_POPV(true)
}

u32 tl_char_copy(char *target, const char *source, const u32 length) {
    TL_STACK_PUSHA("0x%p, 0x%p, %d", target, source, length)

    u32 copied = 0;
    for (u32 i = 0; i < length ; ++i) {
        target[i] = source[i];
        copied++;
    }

    TL_STACK_POPV(copied)
}

TL_INLINE void tl_char_join(char *buffer, const u64 size, const char *str0, const char *str1) {
    TL_STACK_PUSHA("0x%p, %d, %s, %s", buffer, size, str0, str1)
    if (tl_char_length(str0) + tl_char_length(str1) > size) TL_STACK_POP
    sprintf(buffer, "%s%s", str0, str1);
    TL_STACK_POP
}

TL_INLINE void tl_char_from_i32(char *buffer, i32 value, const u8 base) {
    TL_STACK_PUSHA("0x%p, 0x%p, %d, %d", buffer, value, base)

    // check that the base if valid
    if (base < 2 || base > 36) { TL_STACK_POP }

    int tmp_value;
    char *ptr  = buffer;
    char *ptr1 = buffer;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';

    // Reverse the string
    while(ptr1 < ptr) {
        const char tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }

    TL_STACK_POP
}

b8 tl_char_contains(const char *string, const char *token) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, token)

    // const char *s = string;
    // while (*s != '\0') {
    //     if (*s == *token) {
    //         b8 found = true;
    //         const char *sc = s;
    //         const char *t = token;
    //         while (*t != '\0' || *sc != '\0') {
    //             if (*sc != *t) {
    //                 found = false;
    //                 break;
    //             }
    //
    //             sc++;
    //             t++;
    //         }
    //
    //         if (found) TL_STACK_POPV(true)
    //     }
    //
    //     s++;
    // }

    TL_STACK_POPV(strstr(string, token) != NULL)
}
// #####################################################################################################################
//
//                                                     STRING
//
// #####################################################################################################################
struct TLString {
    u64 size;
    u64 length;
    const char *text;
    TLMemoryArena *arena;
    b8 is_view;
};

TL_INLINE static TLString* tl_string_reserve(TLMemoryArena *arena, const u64 size) {
    TL_STACK_PUSHA("0x%p, %d", arena, size)
    TLString *string = tl_memory_alloc(arena, sizeof(struct TLString), TL_MEMORY_STRING);
    string->is_view = false;
    string->arena = arena;
    string->length = 0;
    if (size > 0) {
        string->size = size + 1; // extra byte for NULL character
        string->text = tl_memory_alloc(arena, string->size, TL_MEMORY_STRING);
    }

    TL_STACK_POPV(string)
}

TLMemoryArena* tl_string_arena(TLString *string){
    TL_STACK_PUSHA("0x%p", string)
    TLMemoryArena *arena = string->arena;
    TL_STACK_POPV(arena)
}

TLString* tl_string_clone(TLMemoryArena *arena, const char *string) {
    TL_STACK_PUSHA("0x%p, 0x%p", arena, string)
    TLString *clone = tl_string_reserve(arena, tl_char_length(string));
    clone->length = clone->size - 1;
    tl_memory_copy((void*)clone->text, (void*)string, clone->length);
    TL_STACK_POPV(clone)
}

TLString* tl_string_wrap(TLMemoryArena *arena, const char *string) {
    TL_STACK_PUSHA("0x%p, 0x%p", arena, string)
    TLString *wrap = tl_string_reserve(arena, 0);
    wrap->is_view = true;
    wrap->text = string;
    wrap->length = tl_char_length(string);
    wrap->size = wrap->length + 1;

    TL_STACK_POPV(wrap)
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
TLString* tl_string_from_i32(TLMemoryArena *arena, i32 value, const u8 base) {
    TL_STACK_PUSHA("%d, %d", value, base)

    u32 digits = 0;
    i32 desired = value;
    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);

    TLString* string = tl_string_reserve(arena, digits);

    // check that the base if valid
    if (base < 2 || base > 36) { TL_STACK_POPV(string) }

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

    TL_STACK_POPV(string)
}

void tl_string_join(const TLString *string, const char *other) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, other)
    const u64 length = tl_char_length(other);
    if (length == U32_MAX || length == 0) TLFATAL("Failed to join:\n\n - 0%xs\n - %s", string->text, other)
    ((TLString*)string)->size = string->size + length;

    void *created = tl_memory_alloc(string->arena, string->size, TL_MEMORY_STRING);
    tl_memory_copy(created, (void*)string->text, string->length);
    tl_memory_copy(created + string->length, (void*)other, length);

    ((TLString*)string)->text = created;
    ((TLString*)string)->length = string->size - 1;
    ((TLString*)string)->is_view = false;

    TL_STACK_POP
}

TL_INLINE const char * tl_string(TLString *string) {
    TL_STACK_PUSHA("0x%p", string)
    TL_STACK_POPV(string->text)
}

TLString* tl_string_slice(TLMemoryArena *arena, TLString* string, const u64 offset, const u64 length) {
    TL_STACK_PUSHA("0x%p, 0x%p, %d, %d", arena, string, offset, length)
    //TODO implement tl_string_slice
    TLFATAL("Implementation missing")
    TL_STACK_POPV(NULL)
}

TLString* tl_string_duplicate(TLString *string) {
    TL_STACK_PUSHA("0x%p", string)

    TLString *duplicate = tl_string_reserve(string->arena, string->length);
    duplicate->is_view = false;
    duplicate->length = string->length;
    tl_memory_copy((void*)duplicate->text, (void*)string->text, string->length);

    TL_STACK_POPV(duplicate)
}

TLString* tl_string_view(TLString* string) {
    TL_STACK_PUSHA("0x%p", string)
    TLString *view = tl_string_reserve(string->arena, 0);
    view->is_view = true;
    view->text = string->text;
    view->size = string->size;
    view->length = string->length;
    TL_STACK_POPV(view)
}

u32 tl_string_length(TLString *string) {
    TL_STACK_PUSHA("0x%p", string)
    if (string == NULL) {
        TL_STACK_POPV(-1)
    }
    TL_STACK_POPV(string->length)
}

u32 tl_string_index_of(TLString* string, const char token) {
    TL_STACK_PUSHA("0x%p, %s", string, token)
    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] == token) TL_STACK_POPV(i);
    }
    TL_STACK_POPV(U32_MAX)
}

u32 tl_string_last_index_of(TLString* string, const char token) {
    TL_STACK_PUSHA("0x%p, %c", string, token)
    if (string == NULL) TL_STACK_POPV(U32_MAX);
    if (string->length == 0) TL_STACK_POPV(U32_MAX)

    for (u64 i = string->length - 1; i > 0; --i) {
        if (string->text[i] == token) TL_STACK_POPV(i);
    }
    TLFATAL("Implementation missing")
    TL_STACK_POPV(false)
}

b8 tl_string_start_with(TLString* string, const char* guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) TL_STACK_POPV(false)

    const u64 length = tl_char_length(guess);
    if (length > string->length) TL_STACK_POPV(false)

    for (u64 i = 0; i < length; ++i) {
        if (string->text[i] != guess[i])
            TL_STACK_POPV(false)
    }

    TL_STACK_POPV(true)
}

b8 tl_string_ends_with(TLString* string, const char* guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_ends_with
    TLFATAL("Implementation missing")
    TL_STACK_POPV(false)
}

b8 tl_string_is_view(const TLString* string) {
    TL_STACK_PUSHA("0x%p", string)
    if (string == NULL) {
        TL_STACK_POPV(false)
    }

    TL_STACK_POPV(string->is_view)
}

b8 tl_string_equals(const TLString* string, const char* guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        TL_STACK_POPV(false)
    }

    const u64 length = tl_char_length(guess);
    if (string->length != length) TL_STACK_POPV(false)

    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] != guess[i])
            TL_STACK_POPV(false)
    }

    TL_STACK_POPV(true)
}

b8 tl_string_contains(TLString* string, const char* guess) {
    TL_STACK_PUSHA("0x%p, 0x%p", string, guess)
    b8 contains = tl_char_contains(string->text, guess);
    TL_STACK_POPV(contains)
}