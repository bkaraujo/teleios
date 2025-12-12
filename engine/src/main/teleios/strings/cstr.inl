#ifndef __TELEIOS_STRINGS_CSTR__
#define __TELEIOS_STRINGS_CSTR__

#include "teleios/teleios.h"
#include "teleios/strings/type.inl"

u32 tl_string_length_cstr(const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p", cstr)
    if (cstr == NULL) TL_PROFILER_POP_WITH(0)
    const unsigned length = strlen(cstr);
    TL_PROFILER_POP_WITH(length)
}

b8 tl_string_cstr_ends_with(const char* cstr, const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)
    if (str == NULL || cstr == NULL) TL_PROFILER_POP_WITH(false)

    const u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len < str->length) TL_PROFILER_POP_WITH(false)
    if (cstr_len == str->length) TL_PROFILER_POP_WITH(strcmp(str->data, cstr) == 0)

    const char* pStr = str->data + str->length - 1; // point to the last character
    const char* pCstr = cstr + cstr_len - 1;        // point to the last character

    for (u32 i = 0; i < str->length; i++) {
        if (*pStr-- != *pCstr--) {
            TL_PROFILER_POP_WITH(false)
        }
    }

    TL_PROFILER_POP_WITH(true)
}

#endif