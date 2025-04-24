#ifndef __LIBK_STRING__
#define __LIBK_STRING__

#include "libk/defines.h"
#include "libk/memory.h"

u32 k_char_length(const char *string);
u32 k_char_index_of(const char *string, char token);
u32 k_char_last_index_of(const char *string, char character);
b8 k_char_equals(const char *string, const char *guess);
b8 k_char_start_with(const char *string, const char *guess);
u32 k_char_copy(char *string, const char *source, u32 length);
void k_char_join(char *string, u64 size, const char *str0, const char *str1);
void k_char_from_i32(char *string, i32 value, u8 base);
b8 k_char_contains(const char *string, const char *token);

KAllocator* k_string_allocator(const KString *string);
KString* k_string_clone(KAllocator *allocator, const char *string);
KString* k_string_wrap(KAllocator *allocator, const char *string);
KString* k_string_slice(KAllocator *allocator, const KString *string, u64 offset, u64 length);
KString* k_string_duplicate(const KString *string);
KString* k_string_view(const KString *string);

const char * k_string(KString *string);
u32 k_string_length(KString *string);
u32 k_string_index_of(KString *string, char token);
u32 k_string_last_index_of(KString *string, char token);
b8 k_string_start_with(KString *string, const char *guess);
b8 k_string_ends_with(KString *string, const char *guess);
b8 k_string_is_view(const KString *string);
b8 k_string_equals(const KString *string, const char *guess);
b8 k_string_contains(const KString *string, const char *guess);
KString* k_string_from_i32(KAllocator *allocator, i32 value, u8 base);
void k_string_join(const KString *string, const char *other);

#endif