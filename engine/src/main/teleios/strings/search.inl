#ifndef __TELEIOS_STRINGS_SEARCH__
#define __TELEIOS_STRINGS_SEARCH__

#include "teleios/teleios.h"
#include "teleios/strings/type.inl"

i32 tl_string_index_of_char(const TLString* str, const char ch) {
    TL_PROFILER_PUSH_WITH("%p, '%c'", str, ch)
    if (str == NULL) TL_PROFILER_POP_WITH(-1)
    const char* ptr = strchr(str->data, ch);
    if (ptr == NULL) TL_PROFILER_POP_WITH(-1)
    TL_PROFILER_POP_WITH((i32)(ptr - str->data))
}

i32 tl_string_last_index_of_char(const TLString* str, const char ch) {
    TL_PROFILER_PUSH_WITH("%p, '%c'", str, ch)
    if (str == NULL) TL_PROFILER_POP_WITH(-1)
    const char* ptr = strchr(str->data, ch);
    if (ptr == NULL) TL_PROFILER_POP_WITH(-1)
    TL_PROFILER_POP_WITH((i32)(ptr - str->data))
}

i32 tl_string_index_of(const TLString* str, const TLString* substr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, substr)
    if (str == NULL || substr == NULL) TL_PROFILER_POP_WITH(-1)
    const char* ptr = strstr(str->data, substr->data);
    if (ptr == NULL) TL_PROFILER_POP_WITH(-1)
    TL_PROFILER_POP_WITH((i32)(ptr - str->data))
}

i32 tl_string_index_of_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)
    if (str == NULL || cstr == NULL) TL_PROFILER_POP_WITH(-1)
    const char* ptr = strstr(str->data, cstr);
    if (ptr == NULL) TL_PROFILER_POP_WITH(-1)
    TL_PROFILER_POP_WITH((i32)(ptr - str->data))
}

i32 tl_string_last_index_of(const TLString* str, const TLString* substr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, substr)
    if (str == NULL || substr == NULL) TL_PROFILER_POP_WITH(-1)
    if (substr->length == 0) TL_PROFILER_POP_WITH(-1)

    i32 last_index = -1;
    const char* ptr = str->data;

    while ((ptr = strstr(ptr, substr->data)) != NULL) {
        last_index = (i32)(ptr - str->data);
        ptr += substr->length;
    }

    TL_PROFILER_POP_WITH(last_index)
}

b8 tl_string_contains(const TLString* str, const TLString* substr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, substr)
    const b8 result = (tl_string_index_of(str, substr) != -1);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_string_contains_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)
    const b8 result = (tl_string_index_of_cstr(str, cstr) != -1);
    TL_PROFILER_POP_WITH(result)
}

b8 tl_string_starts_with(const TLString* str, const TLString* prefix) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, prefix)
    if (str == NULL || prefix == NULL) TL_PROFILER_POP_WITH(false)
    if (prefix->length > str->length) TL_PROFILER_POP_WITH(false)
    TL_PROFILER_POP_WITH(strncmp(str->data, prefix->data, prefix->length) == 0)
}

b8 tl_string_starts_with_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)
    if (str == NULL || cstr == NULL) TL_PROFILER_POP_WITH(false)
    const u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len > str->length) TL_PROFILER_POP_WITH(false)
    TL_PROFILER_POP_WITH(strncmp(str->data, cstr, cstr_len) == 0)
}

b8 tl_string_ends_with(const TLString* str, const TLString* suffix) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, suffix)
    if (str == NULL || suffix == NULL) TL_PROFILER_POP_WITH(false)
    if (suffix->length > str->length) TL_PROFILER_POP_WITH(false)

    // Compare character by character from end to beginning
    for (u32 i = 0; i < suffix->length; i++) {
        if (str->data[str->length - suffix->length + i] != suffix->data[i]) {
            TL_PROFILER_POP_WITH(false)
        }
    }

    TL_PROFILER_POP_WITH(true)
}

b8 tl_string_ends_with_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)
    if (str == NULL || cstr == NULL) TL_PROFILER_POP_WITH(false)

    const u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len > str->length) TL_PROFILER_POP_WITH(false)
    if (cstr_len == str->length) TL_PROFILER_POP_WITH(strcmp(str->data, cstr) == 0)

    const char* pStr = str->data + str->length - 1; // point to the last character
    const char* pCstr = cstr + cstr_len - 1;        // point to the last character

    for (u32 i = 0; i < cstr_len; i++) {
        if (*pStr-- != *pCstr--) {
            TL_PROFILER_POP_WITH(false)
        }
    }

    TL_PROFILER_POP_WITH(true)
}

u32 tl_string_count_of_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)
    if (str == NULL || cstr == NULL) TL_PROFILER_POP_WITH(0)

    const u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len == 0) TL_PROFILER_POP_WITH(0)

    u32 count = 0;
    const char* ptr = str->data;

    while ((ptr = strstr(ptr, cstr)) != NULL) {
        count++;
        ptr += cstr_len;
    }

    TL_PROFILER_POP_WITH(count)
}


#endif