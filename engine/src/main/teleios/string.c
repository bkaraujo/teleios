#include "teleios/core.h"
#include "teleios/runtime.h"
#include <string.h>
// #####################################################################################################################
//
//                                                     STRING
//
// #####################################################################################################################
K_INLINE u32 tl_char_length(const char *string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    if (string == NULL ) K_FRAME_POP_WITH(U32_MAX);
    if (*string == '\0') K_FRAME_POP_WITH(0);

    u32 index = 0;

    const char* character = string;
    while (*character++ != '\0') {
        if (index++ == U32_MAX) {
            KFATAL("Failed to find string length")
        }
    }

    K_FRAME_POP_WITH(index)
}

K_INLINE u32 tl_char_last_index_of(const char *string, const char character) {
    K_FRAME_PUSH_WITH("0x%p, %c", string, character)
    u32 index = 0;
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == character) {
            index = i;
        }

        if (++i == U32_MAX) {
            KFATAL("Failed to find string length")
        }
    }

    K_FRAME_POP_WITH(index)
}

K_INLINE u32 tl_char_index_of(const char *string, const char token) {
    K_FRAME_PUSH_WITH("0x%p, %c", string, token)
    const char* s = string;
    for (u32 i = 0; *s != '\0' ; ++s) {
        if (*s == token) {
            K_FRAME_POP_WITH(i)
        }

        if (++i == U32_MAX) {
            KFATAL("Failed to find string length")
        }
    }

    K_FRAME_POP_WITH(U32_MAX)
}

b8 tl_char_equals(const char *string, const char *guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        K_FRAME_POP_WITH(false)
    }

    const u64 length = tl_char_length(string);
    if (tl_char_length(guess) != length) {
        K_FRAME_POP_WITH(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            K_FRAME_POP_WITH(false)
        }
    }

    K_FRAME_POP_WITH(true)
}

b8 tl_char_start_with(const char *string, const char *guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        K_FRAME_POP_WITH(false)
    }

    const u64 length = tl_char_length(guess);
    if (length > tl_char_length(string)) {
        K_FRAME_POP_WITH(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            K_FRAME_POP_WITH(false)
        }
    }

    K_FRAME_POP_WITH(true)
}

u32 tl_char_copy(char *target, const char *source, const u32 length) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, %d", target, source, length)

    u32 copied = 0;
    for (u32 i = 0; i < length ; ++i) {
        target[i] = source[i];
        copied++;
    }

    K_FRAME_POP_WITH(copied)
}

K_INLINE void tl_char_join(char *buffer, const u64 size, const char *str0, const char *str1) {
    K_FRAME_PUSH_WITH("0x%p, %d, %s, %s", buffer, size, str0, str1)
    if (tl_char_length(str0) + tl_char_length(str1) > size) K_FRAME_POP
    sprintf(buffer, "%s%s", str0, str1);
    K_FRAME_POP
}

K_INLINE void tl_char_from_i32(char *buffer, i32 value, const u8 base) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, %d, %d", buffer, value, base)

    // check that the base if valid
    if (base < 2 || base > 36) { K_FRAME_POP }

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

    K_FRAME_POP
}

b8 tl_char_contains(const char *string, const char *token) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, token)

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
    //         if (found) K_FRAME_POP_WITH(true)
    //     }
    //
    //     s++;
    // }

    K_FRAME_POP_WITH(strstr(string, token) != NULL)
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
    KAllocator *allocator;
    b8 is_view;
};

K_INLINE static TLString* tl_string_reserve(KAllocator *allocator, const u64 size) {
    K_FRAME_PUSH_WITH("0x%p, %d", allocator, size)
    TLString *string = k_memory_allocator_alloc(allocator, sizeof(struct TLString), TL_MEMORY_STRING);
    string->is_view = false;
    string->allocator = allocator;
    string->length = 0;
    if (size > 0) {
        string->size = size + 1; // extra byte for NULL character
        string->text = k_memory_allocator_alloc(allocator, string->size, TL_MEMORY_STRING);
    }

    K_FRAME_POP_WITH(string)
}

KAllocator* tl_string_allocator(TLString *string){
    K_FRAME_PUSH_WITH("0x%p", string)
    KAllocator *allocator = string->allocator;
    K_FRAME_POP_WITH(allocator)
}

TLString* tl_string_clone(KAllocator *allocator, const char *string) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", allocator, string)
    TLString *clone = tl_string_reserve(allocator, tl_char_length(string));
    clone->length = clone->size - 1;
    k_memory_copy((void*)clone->text, (void*)string, clone->length);
    K_FRAME_POP_WITH(clone)
}

TLString* tl_string_wrap(KAllocator *allocator, const char *string) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", allocator, string)
    TLString *wrap = tl_string_reserve(allocator, 0);
    wrap->is_view = true;
    wrap->text = string;
    wrap->length = tl_char_length(string);
    wrap->size = wrap->length + 1;

    K_FRAME_POP_WITH(wrap)
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
TLString* tl_string_from_i32(KAllocator *allocator, i32 value, const u8 base) {
    K_FRAME_PUSH_WITH("%d, %d", value, base)

    u32 digits = 0;
    i32 desired = value;
    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);

    TLString* string = tl_string_reserve(allocator, digits);

    // check that the base if valid
    if (base < 2 || base > 36) { K_FRAME_POP_WITH(string) }

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

    K_FRAME_POP_WITH(string)
}

void tl_string_join(const TLString *string, const char *other) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, other)
    const u64 length = tl_char_length(other);
    if (length == U32_MAX || length == 0) KFATAL("Failed to join:\n\n - 0%xs\n - %s", string->text, other)
    ((TLString*)string)->size = string->size + length;

    void *created = k_memory_allocator_alloc(string->allocator, string->size, TL_MEMORY_STRING);
    k_memory_copy(created, (void*)string->text, string->length);
    k_memory_copy(created + string->length, (void*)other, length);

    ((TLString*)string)->text = created;
    ((TLString*)string)->length = string->size - 1;
    ((TLString*)string)->is_view = false;

    K_FRAME_POP
}

K_INLINE const char * tl_string(TLString *string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    K_FRAME_POP_WITH(string->text)
}

TLString* tl_string_slice(KAllocator *allocator, TLString* string, const u64 offset, const u64 length) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, %d, %d", allocator, string, offset, length)
    //TODO implement tl_string_slice
    KFATAL("Implementation missing")
    K_FRAME_POP_WITH(NULL)
}

TLString* tl_string_duplicate(TLString *string) {
    K_FRAME_PUSH_WITH("0x%p", string)

    TLString *duplicate = tl_string_reserve(string->allocator, string->length);
    duplicate->is_view = false;
    duplicate->length = string->length;
    k_memory_copy((void*)duplicate->text, (void*)string->text, string->length);

    K_FRAME_POP_WITH(duplicate)
}

TLString* tl_string_view(TLString* string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    TLString *view = tl_string_reserve(string->allocator, 0);
    view->is_view = true;
    view->text = string->text;
    view->size = string->size;
    view->length = string->length;
    K_FRAME_POP_WITH(view)
}

u32 tl_string_length(TLString *string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    if (string == NULL) {
        K_FRAME_POP_WITH(-1)
    }
    K_FRAME_POP_WITH(string->length)
}

u32 tl_string_index_of(TLString* string, const char token) {
    K_FRAME_PUSH_WITH("0x%p, %s", string, token)
    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] == token) K_FRAME_POP_WITH(i);
    }
    K_FRAME_POP_WITH(U32_MAX)
}

u32 tl_string_last_index_of(TLString* string, const char token) {
    K_FRAME_PUSH_WITH("0x%p, %c", string, token)
    if (string == NULL) K_FRAME_POP_WITH(U32_MAX);
    if (string->length == 0) K_FRAME_POP_WITH(U32_MAX)

    for (u64 i = string->length - 1; i > 0; --i) {
        if (string->text[i] == token) K_FRAME_POP_WITH(i);
    }

    K_FRAME_POP_WITH(U8_MAX)
}

b8 tl_string_start_with(TLString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) K_FRAME_POP_WITH(false)

    const u64 length = tl_char_length(guess);
    if (length > string->length) K_FRAME_POP_WITH(false)

    for (u64 i = 0; i < length; ++i) {
        if (string->text[i] != guess[i])
            K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(true)
}

b8 tl_string_ends_with(TLString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    //TODO implement tl_string_ends_with
    KFATAL("Implementation missing")
    K_FRAME_POP_WITH(false)
}

b8 tl_string_is_view(const TLString* string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    if (string == NULL) {
        K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(string->is_view)
}

b8 tl_string_equals(const TLString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        K_FRAME_POP_WITH(false)
    }

    const u64 length = tl_char_length(guess);
    if (string->length != length) K_FRAME_POP_WITH(false)

    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] != guess[i])
            K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(true)
}

b8 tl_string_contains(TLString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    const b8 contains = tl_char_contains(string->text, guess);
    K_FRAME_POP_WITH(contains)
}