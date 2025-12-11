#ifndef __TELEIOS_CONFIG_GETTERS__
#define __TELEIOS_CONFIG_GETTERS__

#include "teleios/teleios.h"



TLList* tl_config_list(TLString* property) {
    TL_PROFILER_PUSH_WITH("0x%p", property)
    if (property == NULL) {
        TLERROR("Attempted to  list a NULL property");
        TL_PROFILER_POP_WITH(NULL)
    }

    TLDEBUG("Querying properties %s", property);
    TLList* result = tl_map_get(m_properties, property);
    TL_PROFILER_POP_WITH(result)
}

void* tl_config_get(const char* property) {
    TL_PROFILER_PUSH_WITH("0x%p", property)
    if (property == NULL) {
        TLERROR("Attempted to  get a NULL property");
        TL_PROFILER_POP_WITH(NULL)
    }

    TLString* key = tl_string_create(m_allocator, property);
    TLList* list = tl_map_get(m_properties, key);
    tl_string_destroy(key);

    if (list == NULL) {
        TL_PROFILER_POP_WITH(NULL)
    }

    void* value = tl_list_front(list);
    TL_PROFILER_POP_WITH(value)
}

b8 tl_config_get_b8(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)

    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(false)

    TLString* upper = tl_string_to_upper(desired);
    tl_string_destroy(desired);

    const b8 result = tl_string_equals_cstr(upper, "TRUE");
    tl_string_destroy(upper);

    TL_PROFILER_POP_WITH(result);
}

u8 tl_config_get_u8(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const u8 result = tl_number_u8_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

u16 tl_config_get_u16(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const u16 result = tl_number_u16_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

u32 tl_config_get_u32(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const u32 result = tl_number_u32_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

u64 tl_config_get_u64(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const u64 result = tl_number_u64_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

i8 tl_config_get_i8(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const i8 result = tl_number_i8_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

i16 tl_config_get_i16(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const i16 result = tl_number_i16_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

i32 tl_config_get_i32(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const i32 result = tl_number_i32_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

i64 tl_config_get_i64(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0)
    const i64 result = tl_number_i64_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

f32 tl_config_get_f32(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0.0f)
    const f32 result = tl_number_f32_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}

f64 tl_config_get_f64(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)
    TLString* desired = tl_config_get(property);
    if (desired == NULL) TL_PROFILER_POP_WITH(0.0)
    const f64 result = tl_number_f64_from_string(desired);
    tl_string_destroy(desired);
    TL_PROFILER_POP_WITH(result)
}


typedef struct {
    const char *name;
    TLLogLevel value;
} TLLogLevelMap;

TLLogLevel tl_config_get_log_level(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)

    static const TLLogLevelMap logLevelTable[] = {
            { "TL_LOG_LEVEL_VERBOSE",  TL_LOG_LEVEL_VERBOSE },
            { "TL_LOG_LEVEL_TRACE",  TL_LOG_LEVEL_TRACE },
            { "TL_LOG_LEVEL_DEBUG", TL_LOG_LEVEL_DEBUG },
            { "TL_LOG_LEVEL_INFO", TL_LOG_LEVEL_INFO },
            { "TL_LOG_LEVEL_WARN", TL_LOG_LEVEL_WARN },
            { "TL_LOG_LEVEL_ERROR", TL_LOG_LEVEL_ERROR },
            { "TL_LOG_LEVEL_FATAL", TL_LOG_LEVEL_FATAL }
    };

    TLString* desired = tl_config_get(property);
    TLString* upper = tl_string_to_upper(desired);
    tl_string_destroy(desired);

    for (u8 i = 0; i < TL_ARR_LENGTH(logLevelTable); i++) {
        if (tl_string_cstr_ends_with(logLevelTable[i].name, upper)) {
            tl_string_destroy(upper);
            TL_PROFILER_POP_WITH(logLevelTable[i].value);
        }
    }

    TL_PROFILER_POP_WITH(TL_LOG_LEVEL_INFO);
}

typedef struct {
    const char *name;
    TLDisplayResolution value;
} TLResolutionMap;

TLDisplayResolution tl_config_get_display_resolution(const char* property) {
    TL_PROFILER_PUSH_WITH("%s", property)

    static const TLResolutionMap resolutionTable[] = {
            { "TL_DISPLAY_RESOLUTION_SD",  TL_DISPLAY_RESOLUTION_SD },
            { "TL_DISPLAY_RESOLUTION_HD",  TL_DISPLAY_RESOLUTION_HD },
            { "TL_DISPLAY_RESOLUTION_FHD", TL_DISPLAY_RESOLUTION_FHD },
            { "TL_DISPLAY_RESOLUTION_UHD", TL_DISPLAY_RESOLUTION_UHD }
    };

    TLString* desired = tl_config_get(property);
    TLString* upper = tl_string_to_upper(desired);
    tl_string_destroy(desired);

    for (u8 i = 0; i < TL_ARR_LENGTH(resolutionTable); i++) {
        if (tl_string_cstr_ends_with(resolutionTable[i].name, upper)) {
            tl_string_destroy(upper);
            TL_PROFILER_POP_WITH(resolutionTable[i].value);
        }
    }

    TL_PROFILER_POP_WITH(TL_DISPLAY_RESOLUTION_SD);
}

#endif