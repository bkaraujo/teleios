#include "teleios/teleios.h"
#include <ctype.h>

#include "memory_types.inl"

struct TLString {
    char* data;              ///< Null-terminated character array
    u32 length;              ///< Cached string length (excluding null terminator)
    TLAllocator* allocator;  ///< Allocator used for this string
};

// ============================================================================
// String Creation and Destruction
// ============================================================================

TLString* tl_string_create(TLAllocator* allocator, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", allocator, cstr)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (cstr == NULL) TLFATAL("cstr is NULL")

    const u32 length = (u32) strlen(cstr);

    TLString* str = (TLString*)tl_memory_alloc(allocator, TL_MEMORY_STRING, sizeof(TLString));
    str->data = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, length + 1);

    tl_memory_copy(str->data, cstr, length + 1);
    str->length = length;
    str->allocator = allocator;

    TL_PROFILER_POP_WITH(str)
}

TLString* tl_string_create_empty(TLAllocator* allocator) {
    TL_PROFILER_PUSH_WITH("%p", allocator)

    if (allocator == NULL) TLFATAL("allocator is NULL")

    TLString* str = (TLString*)tl_memory_alloc(allocator, TL_MEMORY_STRING, sizeof(TLString));
    str->data = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, 1);
    str->allocator = allocator;

    TL_PROFILER_POP_WITH(str)
}

TLString* tl_string_reserve(TLAllocator* allocator, const u32 capacity) {
    TL_PROFILER_PUSH_WITH("%p, %u", allocator, capacity)

    if (allocator == NULL) TLFATAL("allocator is NULL")

    TLString* str = (TLString*)tl_memory_alloc(allocator, TL_MEMORY_STRING, sizeof(TLString));
    str->data = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, capacity + 1);
    str->allocator = allocator;

    TL_PROFILER_POP_WITH(str)
}

void tl_string_destroy(TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) TLFATAL("str is NULL")
    if (str->data != NULL) {
        tl_memory_free(str->allocator, str->data);
    }

    tl_memory_free(str->allocator, str);

    TL_PROFILER_POP
}

// ============================================================================
// String Properties
// ============================================================================

u32 tl_string_length(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) {
        TL_PROFILER_POP_WITH(0)
    }

    TL_PROFILER_POP_WITH(str->length)
}

char tl_string_char_at(const TLString* str, const u32 index) {
    TL_PROFILER_PUSH_WITH("%p, %u", str, index)

    if (str == NULL || index >= str->length) {
        TL_PROFILER_POP_WITH('\0')
    }

    TL_PROFILER_POP_WITH(str->data[index])
}

const char* tl_string_cstr(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }

    TL_PROFILER_POP_WITH(str->data)
}

b8 tl_string_is_empty(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    const b8 result = (str == NULL || str->length == 0);

    TL_PROFILER_POP_WITH(result)
}

// ============================================================================
// String Comparison
// ============================================================================

b8 tl_string_equals(const TLString* str1, const TLString* str2) {
    TL_PROFILER_PUSH_WITH("%p, %p", str1, str2)

    if (str1 == str2) {
        TL_PROFILER_POP_WITH(true)
    }

    if (str1 == NULL || str2 == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    if (str1->length != str2->length) {
        TL_PROFILER_POP_WITH(false)
    }

    const b8 result = (strcmp(str1->data, str2->data) == 0);

    TL_PROFILER_POP_WITH(result)
}

b8 tl_string_equals_ignore_case(const TLString* str1, const TLString* str2) {
    TL_PROFILER_PUSH_WITH("%p, %p", str1, str2)

    if (str1 == str2) {
        TL_PROFILER_POP_WITH(true)
    }

    if (str1 == NULL || str2 == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    if (str1->length != str2->length) {
        TL_PROFILER_POP_WITH(false)
    }

    const char* p1 = str1->data;
    const char* p2 = str2->data;

    while (*p1 && *p2) {
        if (tolower((unsigned char)*p1) != tolower((unsigned char)*p2)) {
            TL_PROFILER_POP_WITH(false)
        }
        p1++;
        p2++;
    }

    const b8 result = (*p1 == *p2);

    TL_PROFILER_POP_WITH(result)
}

b8 tl_string_equals_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL || cstr == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    const b8 result = (strcmp(str->data, cstr) == 0);

    TL_PROFILER_POP_WITH(result)
}

// ============================================================================
// String Search
// ============================================================================

i32 tl_string_index_of_char(const TLString* str, const char ch) {
    TL_PROFILER_PUSH_WITH("%p, '%c'", str, ch)

    if (str == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const char* ptr = strchr(str->data, ch);
    if (ptr == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const i32 index = (i32)(ptr - str->data);

    TL_PROFILER_POP_WITH(index)
}

i32 tl_string_last_index_of_char(const TLString* str, const char ch) {
    TL_PROFILER_PUSH_WITH("%p, '%c'", str, ch)

    if (str == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const char* ptr = strrchr(str->data, ch);
    if (ptr == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const i32 index = (i32)(ptr - str->data);

    TL_PROFILER_POP_WITH(index)
}

i32 tl_string_index_of(const TLString* str, const TLString* substr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, substr)

    if (str == NULL || substr == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const char* ptr = strstr(str->data, substr->data);
    if (ptr == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const i32 index = (i32)(ptr - str->data);

    TL_PROFILER_POP_WITH(index)
}

i32 tl_string_index_of_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL || cstr == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const char* ptr = strstr(str->data, cstr);
    if (ptr == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    const i32 index = (i32)(ptr - str->data);

    TL_PROFILER_POP_WITH(index)
}

i32 tl_string_last_index_of(const TLString* str, const TLString* substr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, substr)

    if (str == NULL || substr == NULL) {
        TL_PROFILER_POP_WITH(-1)
    }

    if (substr->length == 0) {
        TL_PROFILER_POP_WITH(-1)
    }

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

    if (str == NULL || prefix == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    if (prefix->length > str->length) {
        TL_PROFILER_POP_WITH(false)
    }

    const b8 result = (strncmp(str->data, prefix->data, prefix->length) == 0);

    TL_PROFILER_POP_WITH(result)
}

b8 tl_string_starts_with_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL || cstr == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    const u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len > str->length) {
        TL_PROFILER_POP_WITH(false)
    }

    const b8 result = (strncmp(str->data, cstr, cstr_len) == 0);

    TL_PROFILER_POP_WITH(result)
}

b8 tl_string_ends_with(const TLString* str, const TLString* suffix) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, suffix)

    if (str == NULL || suffix == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    if (suffix->length > str->length) {
        TL_PROFILER_POP_WITH(false)
    }

    const b8 result = (strcmp(str->data + str->length - suffix->length, suffix->data) == 0);

    TL_PROFILER_POP_WITH(result)
}

b8 tl_string_ends_with_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL || cstr == NULL) {
        TL_PROFILER_POP_WITH(false)
    }

    const u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len > str->length) {
        TL_PROFILER_POP_WITH(false)
    }

    const b8 result = (strcmp(str->data + str->length - cstr_len, cstr) == 0);

    TL_PROFILER_POP_WITH(result)
}

// ============================================================================
// String Transformation (returns new TLString)
// ============================================================================

TLString* tl_string_copy(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) TLFATAL("str is NULL")

    TLString* copy = tl_string_create(str->allocator, str->data);

    TL_PROFILER_POP_WITH(copy)
}

TLString* tl_string_substring(const TLString* str, u32 start, u32 end) {
    TL_PROFILER_PUSH_WITH("%p, %u, %u", str, start, end)

    if (str == NULL) TLFATAL("str is NULL")

    if (start > str->length) start = str->length;
    if (end > str->length) end = str->length;
    if (start > end) start = end;

    const u32 substr_len = end - start;

    TLString* substr = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    if (substr == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    substr->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, substr_len + 1);
    if (substr->data == NULL) {
        TLERROR("Failed to allocate memory for substring data")
        tl_memory_free(str->allocator, substr);
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_memory_copy(substr->data, str->data + start, substr_len);
    substr->data[substr_len] = '\0';
    substr->length = substr_len;
    substr->allocator = str->allocator;

    TL_PROFILER_POP_WITH(substr)
}

TLString* tl_string_to_lower(const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p", str)

    if (str == NULL) TLFATAL("str is NULL")

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, str->length + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

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

    if (str == NULL) TLFATAL("str is NULL")

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, str->length + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

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

    if (str == NULL) TLFATAL("str is NULL")

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
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, length + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_memory_copy(result->data, start, length);
    result->data[length] = '\0';
    result->length = length;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_replace_char(const TLString* str, const char old_char, const char new_char) {
    TL_PROFILER_PUSH_WITH("%p, '%c', '%c'", str, old_char, new_char)

    if (str == NULL) TLFATAL("str is NULL")

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, str->length + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

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

    if (str == NULL) TLFATAL("str is NULL")
    if (old_str == NULL) TLFATAL("old_str is NULL")
    if (new_str == NULL) TLFATAL("new_str is NULL")

    i32 index = tl_string_index_of(str, old_str);
    if (index == -1) {
        TL_PROFILER_POP_WITH(tl_string_copy(str))
    }

    const u32 result_len = str->length - old_str->length + new_str->length;

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, result_len + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

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

    if (str == NULL) TLFATAL("str is NULL")
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
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, result_len + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

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
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str1->allocator, TL_MEMORY_STRING, total_len + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str1->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_memory_copy(result->data, str1->data, str1->length);
    tl_memory_copy(result->data + str1->length, str2->data, str2->length);
    result->data[total_len] = '\0';
    result->length = total_len;
    result->allocator = str1->allocator;

    TL_PROFILER_POP_WITH(result)
}

TLString* tl_string_concat_cstr(const TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL) TLFATAL("str is NULL")
    if (cstr == NULL) TLFATAL("cstr is NULL")

    const u32 cstr_len = (u32)strlen(cstr);
    const u32 total_len = str->length + cstr_len;

    TLString* result = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, total_len + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

    tl_memory_copy(result->data, str->data, str->length);
    tl_memory_copy(result->data + str->length, cstr, cstr_len);
    result->data[total_len] = '\0';
    result->length = total_len;
    result->allocator = str->allocator;

    TL_PROFILER_POP_WITH(result)
}

void tl_string_append(TLString* str, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", str, cstr)

    if (str == NULL) TLFATAL("str is NULL")
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
    if (result == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        TL_PROFILER_POP_WITH(NULL)
    }

    result->data = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, total_len + 1);
    if (result->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(allocator, result);
        TL_PROFILER_POP_WITH(NULL)
    }

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

// ============================================================================
// String Splitting
// ============================================================================

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
    if (result == NULL) {
        TLERROR("Failed to allocate memory for split array")
        *out_count = 0;
        TL_PROFILER_POP_WITH(NULL)
    }

    u32 index = 0;
    const char* start = str->data;

    for (const char* p = str->data; *p != '\0'; p++) {
        if (*p == delimiter) {
            const u32 len = (u32)(p - start);

            result[index] = (TLString*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, sizeof(TLString));
            if (result[index] == NULL) {
                TLERROR("Failed to allocate memory for TLString")
                for (u32 i = 0; i < index; i++) {
                    tl_string_destroy(result[i]);
                }
                tl_memory_free(str->allocator, result);
                *out_count = 0;
                TL_PROFILER_POP_WITH(NULL)
            }

            result[index]->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, len + 1);
            if (result[index]->data == NULL) {
                TLERROR("Failed to allocate memory for string data")
                tl_memory_free(str->allocator, result[index]);
                for (u32 i = 0; i < index; i++) {
                    tl_string_destroy(result[i]);
                }
                tl_memory_free(str->allocator, result);
                *out_count = 0;
                TL_PROFILER_POP_WITH(NULL)
            }

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
    if (result[index] == NULL) {
        TLERROR("Failed to allocate memory for TLString")
        for (u32 i = 0; i < index; i++) {
            tl_string_destroy(result[i]);
        }
        tl_memory_free(str->allocator, result);
        *out_count = 0;
        TL_PROFILER_POP_WITH(NULL)
    }

    result[index]->data = (char*)tl_memory_alloc(str->allocator, TL_MEMORY_STRING, len + 1);
    if (result[index]->data == NULL) {
        TLERROR("Failed to allocate memory for string data")
        tl_memory_free(str->allocator, result[index]);
        for (u32 i = 0; i < index; i++) {
            tl_string_destroy(result[i]);
        }
        tl_memory_free(str->allocator, result);
        *out_count = 0;
        TL_PROFILER_POP_WITH(NULL)
    }

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

// ============================================================================
// String Builder Implementation
// ============================================================================

struct TLStringBuilder {
    char* buffer;
    u32 length;
    u32 capacity;
    TLAllocator* allocator;
};

TLStringBuilder* tl_string_builder_create(TLAllocator* allocator, const u32 capacity) {
    TL_PROFILER_PUSH_WITH("%p, %u", allocator, capacity)

    if (allocator == NULL) TLFATAL("allocator is NULL")

    TLStringBuilder* builder = (TLStringBuilder*)tl_memory_alloc(allocator, TL_MEMORY_STRING, sizeof(TLStringBuilder));
    if (builder == NULL) {
        TLERROR("Failed to allocate memory for TLStringBuilder")
        TL_PROFILER_POP_WITH(NULL)
    }

    builder->buffer = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, capacity);
    if (builder->buffer == NULL) {
        TLERROR("Failed to allocate memory for builder buffer")
        tl_memory_free(allocator, builder);
        TL_PROFILER_POP_WITH(NULL)
    }

    builder->capacity = capacity;
    builder->allocator = allocator;

    TL_PROFILER_POP_WITH(builder)
}

static void tl_string_builder_ensure_capacity(TLStringBuilder* builder, u32 required) {
    if (builder->capacity >= required) {
        return;
    }

    u32 new_capacity = builder->capacity * 2;
    while (new_capacity < required) {
        new_capacity *= 2;
    }

    char* new_buffer = (char*)tl_memory_alloc(builder->allocator, TL_MEMORY_STRING, new_capacity);
    if (new_buffer == NULL) {
        TLERROR("Failed to allocate memory for expanded buffer")
        return;
    }

    tl_memory_copy(new_buffer, builder->buffer, builder->length + 1);
    tl_memory_free(builder->allocator, builder->buffer);

    builder->buffer = new_buffer;
    builder->capacity = new_capacity;
}

void tl_string_builder_append(TLStringBuilder* builder, const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p, %p", builder, str)

    if (builder == NULL) TLFATAL("builder is NULL")
    if (str == NULL) TLFATAL("str is NULL")

    tl_string_builder_ensure_capacity(builder, builder->length + str->length + 1);

    tl_memory_copy(builder->buffer + builder->length, str->data, str->length);
    builder->length += str->length;
    builder->buffer[builder->length] = '\0';

    TL_PROFILER_POP
}

void tl_string_builder_append_cstr(TLStringBuilder* builder, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", builder, cstr)

    if (builder == NULL) TLFATAL("builder is NULL")
    if (cstr == NULL) TLFATAL("cstr is NULL")

    const u32 cstr_len = (u32)strlen(cstr);
    tl_string_builder_ensure_capacity(builder, builder->length + cstr_len + 1);

    tl_memory_copy(builder->buffer + builder->length, cstr, cstr_len);
    builder->length += cstr_len;
    builder->buffer[builder->length] = '\0';

    TL_PROFILER_POP
}

void tl_string_builder_append_char(TLStringBuilder* builder, const char ch) {
    TL_PROFILER_PUSH_WITH("%p, '%c'", builder, ch)

    if (builder == NULL) TLFATAL("builder is NULL")

    tl_string_builder_ensure_capacity(builder, builder->length + 2);

    builder->buffer[builder->length] = ch;
    builder->length++;
    builder->buffer[builder->length] = '\0';

    TL_PROFILER_POP
}

TLString* tl_string_builder_build(TLStringBuilder* builder) {
    TL_PROFILER_PUSH_WITH("%p", builder)

    if (builder == NULL) TLFATAL("builder is NULL")

    TLString* str = tl_string_create(builder->allocator, builder->buffer);

    tl_string_builder_destroy(builder);

    TL_PROFILER_POP_WITH(str)
}

void tl_string_builder_clear(TLStringBuilder* builder) {
    TL_PROFILER_PUSH_WITH("%p", builder)

    if (builder == NULL) TLFATAL("builder is NULL")

    builder->buffer[0] = '\0';
    builder->length = 0;

    TL_PROFILER_POP
}

void tl_string_builder_destroy(TLStringBuilder* builder) {
    TL_PROFILER_PUSH_WITH("%p", builder)

    if (builder == NULL) TLFATAL("builder is NULL")

    if (builder->buffer != NULL) {
        tl_memory_free(builder->allocator, builder->buffer);
    }
    tl_memory_free(builder->allocator, builder);

    TL_PROFILER_POP
}
