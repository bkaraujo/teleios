#ifndef __TELEIOS_STRINGS_TYPE__
#define __TELEIOS_STRINGS_TYPE__

#include "teleios/teleios.h"

struct TLString {
    char* data;              ///< Null-terminated character array
    u32 length;              ///< Cached string length (excluding null terminator)
    TLAllocator* allocator;  ///< Allocator used for this string
};

u32 tl_string_length(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)
    if (str == NULL) TL_PROFILER_POP_WITH(0)
    TL_PROFILER_POP_WITH(str->length)
}

char tl_string_char_at(const TLString* str, const u32 index) {
    TL_PROFILER_PUSH_WITH("%p, %u", str, index)
    if (str == NULL || index >= str->length) TL_PROFILER_POP_WITH('\0')
    TL_PROFILER_POP_WITH(str->data[index])
}

const char* tl_string_cstr(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)
    if (str == NULL) TL_PROFILER_POP_WITH(NULL)
    TL_PROFILER_POP_WITH(str->data)
}

b8 tl_string_is_empty(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)
    const b8 result = (str == NULL || str->length == 0);
    TL_PROFILER_POP_WITH(result)
}

#endif