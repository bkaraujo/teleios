#ifndef __TELEIOS_STRINGS_FACTORY__
#define __TELEIOS_STRINGS_FACTORY__

#include "teleios/teleios.h"
#include "teleios/strings/type.inl"

TLString* tl_string_create(TLAllocator* allocator, const char* cstr) {
    TL_PROFILER_PUSH_WITH("%p, %p", allocator, cstr)

    if (allocator == NULL) TLFATAL("allocator is NULL")
    if (cstr == NULL) TLFATAL("cstr is NULL")

    const u32 length = (u32) strlen(cstr);

    TLString* str = (TLString*)tl_memory_alloc(allocator, TL_MEMORY_STRING, sizeof(TLString));
    str->data = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, length + 1);
    str->length = length;
    str->allocator = allocator;
    tl_memory_copy(str->data, cstr, length + 1);

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

#endif