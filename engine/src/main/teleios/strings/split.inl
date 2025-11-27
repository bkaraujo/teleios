#ifndef __TELEIOS_STRINGS_SPLIT__
#define __TELEIOS_STRINGS_SPLIT__

#include "teleios/teleios.h"
#include "teleios/strings/type.inl"

TLString** tl_string_split(const TLString* str, const char delimiter, u32* out_count) {
    TL_PROFILER_PUSH_WITH("%p, '%c', %p", str, delimiter, out_count)

    if (str == NULL) TLFATAL("str is NULL")
    if (out_count == NULL) TLFATAL("out_count is NULL")

    // Count delimiters
    u32 count = 1;
    for (u32 i = 0; i < str->length; i++) {
        if (str->data[i] == delimiter) count++;
    }

    TLString** result = (TLString**)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, count * sizeof(TLString*));

    u32 index = 0;
    const char* start = str->data;

    for (const char* p = str->data; *p != '\0'; p++) {
        if (*p == delimiter) {
            const u32 len = (u32)(p - start);

            result[index] = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
            result[index]->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, len + 1);

            tl_memory_copy(result[index]->data, start, len);
            result[index]->data[len] = '\0';
            result[index]->length = len;
            result[index]->allocator = str->allocator;

            index++;
            start = p + 1;
        }
    }

    // Handle last segment
    const u32 len = (u32)(str->data + str->length - start);
    result[index] = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result[index]->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, len + 1);

    tl_memory_copy(result[index]->data, start, len);
    result[index]->data[len] = '\0';
    result[index]->length = len;
    result[index]->allocator = str->allocator;

    *out_count = count;

    TL_PROFILER_POP_WITH(result)
}

void tl_string_split_destroy(TLString** strings, u32 count) {
    TL_PROFILER_PUSH_WITH("%p, %u", strings, count)

    if (strings == NULL) TLFATAL("strings is NULL")

    TLAllocator* allocator = strings[0]->allocator;

    for (u32 i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            tl_string_destroy(strings[i]);
        }
    }
    tl_memory_free(allocator, strings);

    TL_PROFILER_POP
}

#endif