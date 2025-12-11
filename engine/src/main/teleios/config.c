#include "teleios/teleios.h"

static TLAllocator* m_allocator;
static TLMap* m_properties;

static void tl_serializer_walk();

b8 tl_config_initialize() {
    TL_PROFILER_PUSH
    m_allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    m_properties = tl_map_create(m_allocator, 32, false);
    tl_serializer_walk();

    tl_logger_set_level(tl_config_get_log_level("teleios.logging.level"));
    TL_PROFILER_POP_WITH(true)
}

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

TLScene* tl_config_get_scene(const TLString* name) {
    TL_PROFILER_PUSH_WITH("name=%x%p", name)



    TL_PROFILER_POP_WITH(NULL)
}

b8 tl_config_terminate(void) {
    TL_PROFILER_PUSH

    if (m_properties != NULL) {
        tl_map_destroy(m_properties);
        m_properties = NULL;
    }

    if (m_allocator != NULL) {
        tl_memory_allocator_destroy(m_allocator);
        m_allocator = NULL;
    }

    TL_PROFILER_POP_WITH(true)
}

// #####################################################################################################################
//
//                                                     SERIALIZER
//
// #####################################################################################################################
#include <yaml.h>

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) TLFATAL("Unexpect token")

typedef struct {
    TLString* name;
    u32 sequence;
} TLTuple;

// Helper: Build path from segments using cached iterator (OPTIMIZATION #1)
// Uses tl_iterator_resync() to avoid creating new iterators on each call
static void build_path_from_segments(TLStringBuilder* builder, TLIterator* cached_iter) {
    if (cached_iter == NULL) return;

    tl_iterator_resync(cached_iter);
    while (tl_iterator_has_next(cached_iter)) {
        const TLString* segment = (TLString*)tl_iterator_next(cached_iter);
        tl_string_builder_append(builder, segment);
        tl_string_builder_append_cstr(builder, ".");
    }
}

static void tl_serializer_walk() {
    TL_PROFILER_PUSH
    FILE* file = fopen("application.yml", "r");
    if (file == NULL) TLFATAL("Failed to open application.yml");

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); TLFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    TLAllocator *allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    TLMap *sequences = tl_map_create(allocator, 8, false);  // OPTIMIZATION #2: Hash map for O(1) sequence lookup
    TLList *path_segments = tl_list_create(allocator, false);  // Stack of TLString* representing current path

    yaml_token_t token;
    TLString* current_key = NULL;  // Stores current KEY token before value/nested block
    TLStringBuilder* builder = tl_string_builder_create(allocator, 512);

    // OPTIMIZATION #1: Cached iterator for path_segments (reused with resync)
    TLIterator* path_iter = tl_list_iterator(path_segments);

    // OPTIMIZATION #3: Cache current path to avoid rebuilding
    TLStringBuilder* path_cache = tl_string_builder_create(allocator, 256);
    u32 path_depth = 0;

    do {
        yaml_parser_scan(&parser, &token);

        switch(token.type) {
            default: continue;
            case YAML_NO_TOKEN: TLFATAL("Malformed YAML token");
            // #########################################################################################################
            // YAML_BLOCK_SEQUENCE_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_SEQUENCE_START_TOKEN: {
                // Add current_key to path (it's a sequence container)
                if (current_key != NULL) {
                    tl_list_push_back(path_segments, current_key);
                    // OPTIMIZATION #3: Update cached path incrementally
                    tl_string_builder_append(path_cache, current_key);
                    tl_string_builder_append_cstr(path_cache, ".");
                    path_depth++;
                    current_key = NULL;
                }

                // Create tuple for sequence indexing
                TLTuple *tuple = tl_memory_alloc(allocator, TL_MEMORY_SERIALIZER, sizeof(TLTuple));
                tuple->sequence = 0;

                // OPTIMIZATION #1: Build tuple name using cached iterator with resync
                tl_string_builder_clear(builder);
                build_path_from_segments(builder, path_iter);
                tuple->name = tl_string_builder_build(builder);

                // OPTIMIZATION #2: Store in hash map for O(1) lookup
                tl_map_put(sequences, tuple->name, tuple);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_MAPPING_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_MAPPING_START_TOKEN: {
                // If current_key exists, add it to path (it's a nested mapping)
                if (current_key != NULL) {
                    tl_list_push_back(path_segments, current_key);
                    // OPTIMIZATION #3: Update cached path incrementally
                    tl_string_builder_append(path_cache, current_key);
                    tl_string_builder_append_cstr(path_cache, ".");
                    path_depth++;
                    current_key = NULL;
                }
            } break;
            // #########################################################################################################
            // YAML_BLOCK_ENTRY_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_ENTRY_TOKEN: {
                // OPTIMIZATION #1: Build current path using cached iterator with resync
                tl_string_builder_clear(builder);
                build_path_from_segments(builder, path_iter);
                TLString* current_path = tl_string_builder_build(builder);

                // OPTIMIZATION #2: O(1) hash map lookup instead of O(n) linear search
                TLList* tuple_list = tl_map_get(sequences, current_path);
                TLString* index_string = NULL;

                if (tuple_list != NULL && tl_list_size(tuple_list) > 0) {
                    TLTuple* tuple = (TLTuple*)tl_list_front(tuple_list);
                    index_string = tl_number_i32_to_char(allocator, (i32)tuple->sequence, 10);
                    tuple->sequence++;
                }

                // Add index to path_segments
                if (index_string != NULL) {
                    tl_list_push_back(path_segments, index_string);
                    // OPTIMIZATION #3: Update cached path incrementally
                    tl_string_builder_append(path_cache, index_string);
                    tl_string_builder_append_cstr(path_cache, ".");
                    path_depth++;
                }
            } continue;
            // #########################################################################################################
            // YAML_KEY_TOKEN
            // #########################################################################################################
            case YAML_KEY_TOKEN: {
                EXPECT(YAML_SCALAR_TOKEN)

                // Store key - will be added to path if followed by BLOCK_MAPPING_START/BLOCK_SEQUENCE_START
                // or used as final property name if followed by YAML_SCALAR_TOKEN (value)
                current_key = tl_string_create(allocator, (char*)token.data.scalar.value);
            } break;
            // #########################################################################################################
            // YAML_SCALAR_TOKEN
            // #########################################################################################################
            case YAML_SCALAR_TOKEN: {
                // OPTIMIZATION #1: Use cached iterator with resync
                // OPTIMIZATION #3: Cache is maintained but we rebuild each time
                tl_string_builder_clear(builder);
                build_path_from_segments(builder, path_iter);

                if (current_key != NULL) {
                    tl_string_builder_append(builder, current_key);
                    current_key = NULL;  // Reset key after use
                }

                TLString* property = tl_string_builder_build(builder);

                tl_map_put(
                    m_properties,
                    tl_string_create(m_allocator, tl_string_cstr(property)),
                    tl_string_create(m_allocator, (char*)token.data.scalar.value)
                );
            } break;
            // #########################################################################################################
            // YAML_BLOCK_END_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_END_TOKEN: {
                // Pop last segment from path
                if (!tl_list_is_empty(path_segments)) {
                    tl_list_pop_back(path_segments);

                    // OPTIMIZATION #3: Rebuild cache after pop (cheaper than tracking segment lengths)
                    if (path_depth > 0) {
                        path_depth--;
                        tl_string_builder_clear(path_cache);
                        build_path_from_segments(path_cache, path_iter);
                    }
                }
            } break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    tl_iterator_destroy(path_iter);  // Cleanup cached iterator
    tl_list_destroy(path_segments);
    tl_map_destroy(sequences);  // Changed from tl_list_destroy
    tl_string_builder_destroy(builder);
    tl_string_builder_destroy(path_cache);  // Added cleanup for path cache
    tl_memory_allocator_destroy(allocator);

    TL_PROFILER_POP
}
