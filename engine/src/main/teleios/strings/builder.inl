#ifndef __TELEIOS_STRINGS_BUILDER__
#define __TELEIOS_STRINGS_BUILDER__

#include "teleios/teleios.h"
#include "teleios/strings/type.inl"

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
    builder->buffer = (char*)tl_memory_alloc(allocator, TL_MEMORY_STRING, capacity);

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
    tl_memory_copy(new_buffer, builder->buffer, builder->length + 1);
    tl_memory_free(builder->allocator, builder->buffer);

    builder->buffer = new_buffer;
    builder->capacity = new_capacity;
}

void tl_string_builder_append(TLStringBuilder* builder, const TLString* str) {
    TL_PROFILER_PUSH_WITH("%p, %p", builder, str)

    if (builder == NULL) TLFATAL("builder is NULL")
    if (str == NULL) TLFATAL("Attempted to usa a NULL TLString")

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


#endif