#include "libk/libk.h"
// #####################################################################################################################
//
//                                                     STRING
//
// #####################################################################################################################
u32 k_char_length(const char *string) {
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

u32 k_char_last_index_of(const char *string, const char character) {
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

u32 k_char_index_of(const char *string, const char token) {
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

b8 k_char_equals(const char *string, const char *guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        K_FRAME_POP_WITH(false)
    }

    const u64 length = k_char_length(string);
    if (k_char_length(guess) != length) {
        K_FRAME_POP_WITH(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            K_FRAME_POP_WITH(false)
        }
    }

    K_FRAME_POP_WITH(true)
}

b8 k_char_start_with(const char *string, const char *guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        K_FRAME_POP_WITH(false)
    }

    const u64 length = k_char_length(guess);
    if (length > k_char_length(string)) {
        K_FRAME_POP_WITH(false)
    }

    for (u64 i = 0; i < length; ++i) {
        if (string[i] != guess[i]) {
            K_FRAME_POP_WITH(false)
        }
    }

    K_FRAME_POP_WITH(true)
}

u32 k_char_copy(char *target, const char *source, const u32 length) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, %d", target, source, length)

    u32 copied = 0;
    for (u32 i = 0; i < length ; ++i) {
        target[i] = source[i];
        copied++;
    }

    K_FRAME_POP_WITH(copied)
}

void k_char_join(char *buffer, const u64 size, const char *str0, const char *str1) {
    K_FRAME_PUSH_WITH("0x%p, %d, %s, %s", buffer, size, str0, str1)
    if (k_char_length(str0) + k_char_length(str1) > size) K_FRAME_POP
    sprintf(buffer, "%s%s", str0, str1);
    K_FRAME_POP
}

void k_char_from_i32(char *buffer, i32 value, const u8 base) {
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

b8 k_char_contains(const char *string, const char *token) {
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
struct KString {
    u64 size;
    u64 length;
    const char *text;
    KAllocator *allocator;
    b8 is_view;
};

static KString* k_string_reserve(KAllocator *allocator, const u64 size) {
    K_FRAME_PUSH_WITH("0x%p, %d", allocator, size)
    KString *string = k_memory_allocator_alloc(allocator, sizeof(struct KString), K_MEMORY_STRING);
    string->is_view = false;
    string->allocator = allocator;
    string->length = 0;
    if (size > 0) {
        string->size = size + 1; // extra byte for NULL character
        string->text = k_memory_allocator_alloc(allocator, string->size, K_MEMORY_STRING);
    }

    K_FRAME_POP_WITH(string)
}

KAllocator* k_string_allocator(const KString *string){
    K_FRAME_PUSH_WITH("0x%p", string)
    KAllocator *allocator = string->allocator;
    K_FRAME_POP_WITH(allocator)
}

KString* k_string_clone(KAllocator *allocator, const char *string) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", allocator, string)
    KString *clone = k_string_reserve(allocator, k_char_length(string));
    clone->length = clone->size - 1;
    k_memory_copy((void*)clone->text, (void*)string, clone->length);
    K_FRAME_POP_WITH(clone)
}

KString* k_string_wrap(KAllocator *allocator, const char *string) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", allocator, string)
    KString *wrap = k_string_reserve(allocator, 0);
    wrap->is_view = true;
    wrap->text = string;
    wrap->length = k_char_length(string);
    wrap->size = wrap->length + 1;

    K_FRAME_POP_WITH(wrap)
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
KString* k_string_from_i32(KAllocator *allocator, i32 value, const u8 base) {
    K_FRAME_PUSH_WITH("%d, %d", value, base)

    u32 digits = 0;
    i32 desired = value;
    do {
        digits++;
        desired = desired / 10;
    } while(desired > 0);

    KString* string = k_string_reserve(allocator, digits);

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

void k_string_join(const KString *string, const char *other) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, other)
    const u64 length = k_char_length(other);
    if (length == U32_MAX || length == 0) KFATAL("Failed to join:\n\n - 0%xs\n - %s", string->text, other)
    ((KString*)string)->size = string->size + length;

    char *created = k_memory_allocator_alloc(string->allocator, string->size, K_MEMORY_STRING);
    k_memory_copy(created, string->text, string->length);
    k_memory_copy(created + string->length, other, length);

    ((KString*)string)->text = created;
    ((KString*)string)->length = string->size - 1;
    ((KString*)string)->is_view = false;

    K_FRAME_POP
}

const char * k_string(KString *string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    K_FRAME_POP_WITH(string->text)
}

KString* k_string_slice(KAllocator *allocator, const KString* string, const u64 offset, const u64 length) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p, %d, %d", allocator, string, offset, length)
    //TODO implement k_string_slice
    KFATAL("Implementation missing")
    K_FRAME_POP_WITH(NULL)
}

KString* k_string_duplicate(const KString *string) {
    K_FRAME_PUSH_WITH("0x%p", string)

    KString *duplicate = k_string_reserve(string->allocator, string->length);
    duplicate->is_view = false;
    duplicate->length = string->length;
    k_memory_copy((void*)duplicate->text, (void*)string->text, string->length);

    K_FRAME_POP_WITH(duplicate)
}

KString* k_string_view(const KString* string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    KString *view = k_string_reserve(string->allocator, 0);
    view->is_view = true;
    view->text = string->text;
    view->size = string->size;
    view->length = string->length;
    K_FRAME_POP_WITH(view)
}

u32 k_string_length(KString *string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    if (string == NULL) {
        K_FRAME_POP_WITH(-1)
    }
    K_FRAME_POP_WITH(string->length)
}

u32 k_string_index_of(KString* string, const char token) {
    K_FRAME_PUSH_WITH("0x%p, %s", string, token)
    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] == token) K_FRAME_POP_WITH(i);
    }
    K_FRAME_POP_WITH(U32_MAX)
}

u32 k_string_last_index_of(KString* string, const char token) {
    K_FRAME_PUSH_WITH("0x%p, %c", string, token)
    if (string == NULL) K_FRAME_POP_WITH(U32_MAX);
    if (string->length == 0) K_FRAME_POP_WITH(U32_MAX)

    for (u64 i = string->length - 1; i > 0; --i) {
        if (string->text[i] == token) K_FRAME_POP_WITH(i);
    }

    K_FRAME_POP_WITH(U8_MAX)
}

b8 k_string_start_with(KString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) K_FRAME_POP_WITH(false)

    const u64 length = k_char_length(guess);
    if (length > string->length) K_FRAME_POP_WITH(false)

    for (u64 i = 0; i < length; ++i) {
        if (string->text[i] != guess[i])
            K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(true)
}

b8 k_string_ends_with(KString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    //TODO implement k_string_ends_with
    KFATAL("Implementation missing")
    K_FRAME_POP_WITH(false)
}

b8 k_string_is_view(const KString* string) {
    K_FRAME_PUSH_WITH("0x%p", string)
    if (string == NULL) {
        K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(string->is_view)
}

b8 k_string_equals(const KString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    if (string == NULL || guess == NULL) {
        K_FRAME_POP_WITH(false)
    }

    const u64 length = k_char_length(guess);
    if (string->length != length) K_FRAME_POP_WITH(false)

    for (u64 i = 0; i < string->length; ++i) {
        if (string->text[i] != guess[i])
            K_FRAME_POP_WITH(false)
    }

    K_FRAME_POP_WITH(true)
}

b8 k_string_contains(const KString* string, const char* guess) {
    K_FRAME_PUSH_WITH("0x%p, 0x%p", string, guess)
    const b8 contains = k_char_contains(string->text, guess);
    K_FRAME_POP_WITH(contains)
}