#ifndef __TELEIOS_STRINGS_COMPARE__
#define __TELEIOS_STRINGS_COMPARE__

#include "teleios/teleios.h"
#include "teleios/strings/type.inl"

b8 tl_string_equals(const TLString* str1, const TLString* str2) {
    TL_PROFILER_PUSH_WITH("%p, %p", str1, str2)
    if (str1 == str2) TL_PROFILER_POP_WITH(true)
    if (str1 == NULL || str2 == NULL) TL_PROFILER_POP_WITH(false)
    if (str1->length != str2->length) TL_PROFILER_POP_WITH(false)
    TL_PROFILER_POP_WITH(strcmp(str1->data, str2->data) == 0)
}

b8 tl_string_equals_ignore_case(const TLString* str1, const TLString* str2) {
    TL_PROFILER_PUSH_WITH("%p, %p", str1, str2)
    if (str1 == str2) TL_PROFILER_POP_WITH(true)
    if (str1 == NULL || str2 == NULL) TL_PROFILER_POP_WITH(false)
    if (str1->length != str2->length) TL_PROFILER_POP_WITH(false)

    const char* p1 = str1->data;
    const char* p2 = str2->data;

    while (*p1 && *p2) {
        if (tolower((unsigned char)*p1) != tolower((unsigned char)*p2)) {
            TL_PROFILER_POP_WITH(false)
        }

        p1++;
        p2++;
    }

    TL_PROFILER_POP_WITH(*p1 == *p2)
}

b8 tl_string_equals_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)
    if (str == NULL || cstr == NULL) TL_PROFILER_POP_WITH(false)
    TL_PROFILER_POP_WITH(strcmp(str->data, cstr) == 0)
}

#endif