#ifndef __TELEIOS_STRINGS_TRANSFORM__
#define __TELEIOS_STRINGS_TRANSFORM__

#include "teleios/teleios.h"
#include "teleios/strings/type.inl"

TLString* tl_string_copy(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)
    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")
    TLString* copy = tl_string_create(str->allocator, str->data);
    TL_PROFILER_POP_WITH(copy)
}

TLString* tl_string_substring(const TLString* str, u32 start, u32 end) {
    TL_PROFILER_PUSH_WITH("%p, %u, %u", str, start, end)
    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")

    if (start > str->length) start = str->length;
    if (end > str->length) end = str->length;
    if (start > end) start = end;

    const u32 substr_len = end - start;

    TLString* substr = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    substr->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, substr_len + 1);

    tl_memory_copy(substr->data, str->data + start, substr_len);
    substr->data[substr_len] = '\0';
    substr->length = substr_len;
    substr->allocator = str->allocator;

    TL_PROFILER_POP_WITH(substr)
}

TLString* tl_string_to_lower(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, str->length + 1);

    for (u32 i = 0; i < str->length; i++) {
        result->data[i] = (char)tolower((unsigned char)str->data[i]);
    }

    result->data[str->length] = '\0';
    result->length = str->length;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_to_upper(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, str->length + 1);

    for (u32 i = 0; i < str->length; i++) {
        result->data[i] = (char)toupper((unsigned char)str->data[i]);
    }
    result->data[str->length] = '\0';
    result->length = str->length;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_trim(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")

    const char* start = str->data;
    const char* end = str->data + str->length - 1;

    // Skip leading whitespace
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // If all whitespace, return empty string
    if (*start == '\0') {
        TL_PROFILER_POP_WITH(tl_string_create_empty(str->allocator))
    }

    // Skip trailing whitespace
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    const u32 length = (u32)(end - start + 1);

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, length + 1);

    tl_memory_copy(result->data, start, length);
    result->data[length] = '\0';
    result->length = length;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_replace_char(const TLString* str, const char old_char, const char new_char) {
    TL_PROFILER_PUSH_WITH("%p, '%c', '%c'", str, old_char, new_char)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, str->length + 1);

    for (u32 i = 0; i < str->length; i++) {
        result->data[i] = (str->data[i] == old_char) ? new_char : str->data[i];
    }
    result->data[str->length] = '\0';
    result->length = str->length;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_replace_first(const TLString* str, const TLString* old_str, const TLString* new_str) {
    TL_PROFILER_PUSH_WITH("%p, %p, %p", str, old_str, new_str)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")
    if (old_str == NULL) TLFATAL("old_str is NULL")
    if (new_str == NULL) TLFATAL("new_str is NULL")

    i32 index = tl_string_index_of(str, old_str);
    if (index == -1) {
        TL_PROFILER_POP_WITH(tl_string_copy(str))
    }

    const u32 result_len = str->length - old_str->length + new_str->length;

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, result_len + 1);

    tl_memory_copy(result->data, str->data, index);
    tl_memory_copy(result->data + index, new_str->data, new_str->length);
    tl_memory_copy(result->data + index + new_str->length, str->data + index + old_str->length, str->length - index - old_str->length);
    result->data[result_len] = '\0';
    result->length = result_len;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_replace_all(const TLString* str, const TLString* old_str, const TLString* new_str) {
    TL_PROFILER_PUSH_WITH("%p, %p, %p", str, old_str, new_str)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")
    if (old_str == NULL) TLFATAL("old_str is NULL")
    if (new_str == NULL) TLFATAL("new_str is NULL")

    if (old_str->length == 0) {
        TL_PROFILER_POP_WITH(tl_string_copy(str))
    }

    // Count occurrences
    u32 count = 0;
    const char* tmp = str->data;
    while ((tmp = strstr(tmp, old_str->data)) != NULL) {
        count++;
        tmp += old_str->length;
    }

    if (count == 0) {
        TL_PROFILER_POP_WITH(tl_string_copy(str))
    }

    const u32 result_len = str->length + count * (new_str->length - old_str->length);

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, result_len + 1);

    char* dest = result->data;
    const char* src = str->data;

    while ((tmp = strstr(src, old_str->data)) != NULL) {
        const u32 prefix_len = (u32)(tmp - src);
        tl_memory_copy(dest, src, prefix_len);
        dest += prefix_len;

        tl_memory_copy(dest, new_str->data, new_str->length);
        dest += new_str->length;

        src = tmp + old_str->length;
    }

    const u32 remaining = (u32)(str->data + str->length - src);
    tl_memory_copy(dest, src, remaining);
    result->data[result_len] = '\0';
    result->length = result_len;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_concat(const TLString* str1, const TLString* str2) {
    TL_PROFILER_PUSH_WITH("%p, %p", str1, str2)

    if (str1 == NULL) TLFATAL("str1 is NULL")
    if (str2 == NULL) TLFATAL("str2 is NULL")

    const u32 total_len = str1->length + str2->length;

    TLString* result = (TLString*)tl_memory_alloc(str1->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str1->allocator, TL_MEMORY_STRING, total_len + 1);

    tl_memory_copy(result->data, str1->data, str1->length);
    tl_memory_copy(result->data + str1->length, str2->data, str2->length);
    result->data[total_len] = '\0';
    result->length = total_len;
    result->allocator = str1->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_concat_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")
    if (cstr == NULL) TLFATAL("cstr is NULL")

    const u32 cstr_len = (u32)strlen(cstr);
    const u32 total_len = str->length + cstr_len;

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, total_len + 1);

    tl_memory_copy(result->data, str->data, str->length);
    tl_memory_copy(result->data + str->length, cstr, cstr_len);
    result->data[total_len] = '\0';
    result->length = total_len;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

void tl_string_append(TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")
    if (cstr == NULL) TLFATAL("cstr is NULL")

    const u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len == 0) {
        TL_PROFILER_POP
    }

    const u32 new_length = str->length + cstr_len;

    // Reallocate buffer to accommodate new content
    char* new_data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, new_length + 1);

    // Copy existing content and append new content
    tl_memory_copy(new_data, str->data, str->length);
    tl_memory_copy(new_data + str->length, cstr, cstr_len);
    new_data[new_length] = '\0';

    // Free old buffer and update string
    tl_memory_free(str->allocator, str->data);
    str->data = new_data;
    str->length = new_length;

    TL_PROFILER_POP
}

TLString* tl_string_concat_multiple(TLAllocator* allocator, const TLString** strings) {
    TL_PROFILER_PUSH_WITH("%p, %p", allocator, strings)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (strings == NULL) TLFATAL("strings is NULL")

    // Calculate total length
    u32 total_len = 0;
    for (u32 i = 0; strings[i] != NULL; i++) {
        total_len += strings[i]->length;
    }

    TLString* result = (TLString*)tl_memory_alloc(allocator, TL_MEMORY_STRING, sizeof(TLString));
    result->data = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, total_len + 1);

    char* dest = result->data;
    for (u32 i = 0; strings[i] != NULL; i++) {
        tl_memory_copy(dest, strings[i]->data, strings[i]->length);
        dest += strings[i]->length;
    }
    *dest = '\0';
    result->length = total_len;
    result->allocator = allocator;

    TL_PROFILER_POP_WITH(result)
}


#endif