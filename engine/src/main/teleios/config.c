#include "container_types.inl"
#include "teleios/teleios.h"

static TLAllocator* m_dynamic;
static TLAllocator* m_allocator;
static TLMap* m_properties;

static void tl_serializer_walk();

static void tl_config_print_all();

b8 tl_config_initialize() {
    TL_PROFILER_PUSH

    m_dynamic = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
    m_allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    m_properties = tl_map_create(m_allocator, 32);

    tl_serializer_walk();
    // tl_config_print_all();

    TL_PROFILER_POP_WITH(true)
}

void* tl_config_get(const char* property) {
    TL_PROFILER_PUSH_WITH("0x%p", property)
    if (property == NULL) {
        TLWARN("Attempting to get a NULL property");
        TL_PROFILER_POP_WITH(NULL)
    }

    TLDEBUG("Querying property %s", property);
    TLString* key = tl_string_create(m_allocator, property);
    TLList* list = tl_map_get(m_properties, key);
    tl_string_destroy(key);

    void* value = tl_list_front(list);

    TL_PROFILER_POP_WITH(value)
}

TLList* tl_config_list(TLString* property) {
    TL_PROFILER_PUSH_WITH("0x%p", property)
    if (property == NULL) {
        TLWARN("Attempting to list a NULL property");
        TL_PROFILER_POP_WITH(NULL)
    }

    TLDEBUG("Querying properties %s", property);
    TLList* result = tl_map_get(m_properties, property);
    TL_PROFILER_POP_WITH(result)
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

#define TL_YAML_PROPERTY_MAX_SIZE   1024

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) TLFATAL("Unexpect token")

typedef struct {
    TLString* name;
    u32 sequence;
} TLTuple;

static void tl_serializer_walk() {
    TL_PROFILER_PUSH
    FILE* file = fopen("application.yml", "r");
    if (file == NULL) TLFATAL("Failed to open application.yml");

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); TLFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    TLAllocator *allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    TLList *sequences = tl_list_create(allocator);
    TLList *path_segments = tl_list_create(allocator);  // Stack of TLString* representing current path

    yaml_token_t token;
    TLString* current_key = NULL;  // Stores current KEY token before value/nested block
    TLStringBuilder* builder = tl_string_builder_create(allocator, 512);

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
                    current_key = NULL;
                }

                // Create tuple for sequence indexing
                TLTuple *tuple = tl_memory_alloc(allocator, TL_MEMORY_SERIALIZER, sizeof(TLTuple));

                // Build tuple name: path + "."
                tl_string_builder_clear(builder);
                TLIterator* iterator = tl_list_iterator(path_segments);
                for (TLString* segment = tl_iterator_next(iterator); segment != NULL; segment = tl_iterator_next(iterator)) {
                    tl_string_builder_append(builder, segment);
                    tl_string_builder_append_cstr(builder, ".");
                }
                tl_iterator_destroy(iterator);

                tuple->name = tl_string_builder_build(builder);

                tl_list_push_back(sequences, tuple);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_MAPPING_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_MAPPING_START_TOKEN: {
                // If current_key exists, add it to path (it's a nested mapping)
                if (current_key != NULL) {
                    tl_list_push_back(path_segments, current_key);
                    current_key = NULL;
                }
            } break;
            // #########################################################################################################
            // YAML_BLOCK_ENTRY_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_ENTRY_TOKEN: {
                // Build current path to find matching sequence
                tl_string_builder_clear(builder);
                TLIterator* iterator = tl_list_iterator(path_segments);
                for (TLString* segment = tl_iterator_next(iterator); segment != NULL; segment = tl_iterator_next(iterator)) {
                    tl_string_builder_append(builder, segment);
                    tl_string_builder_append_cstr(builder, ".");
                }
                tl_iterator_destroy(iterator);

                TLString* current_path = tl_string_builder_build(builder);

                // Find matching sequence and increment
                TLString* index_string = NULL;
                TLIterator* seq_iterator = tl_list_iterator(sequences);
                for (TLTuple* tuple = tl_iterator_next(seq_iterator); tuple != NULL; tuple = tl_iterator_next(seq_iterator)) {
                    if (tl_string_equals(tuple->name, current_path)) {
                        index_string = tl_number_i32_to_char(allocator, (i32)tuple->sequence, 10);
                        tuple->sequence++;
                        break;
                    }
                }
                tl_iterator_destroy(seq_iterator);

                // Add index to path_segments
                if (index_string != NULL) {
                    tl_list_push_back(path_segments, index_string);
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
                // Build property: path_segments + current_key

                tl_string_builder_clear(builder);
                TLIterator* iterator = tl_list_iterator(path_segments);
                for (TLString* segment = tl_iterator_next(iterator); segment != NULL; segment = tl_iterator_next(iterator)) {
                    tl_string_builder_append(builder, segment);
                    tl_string_builder_append_cstr(builder, ".");
                }
                tl_iterator_destroy(iterator);

                if (current_key != NULL) {
                    tl_string_builder_append(builder, current_key);
                }

                TLString* property = tl_string_builder_build(builder);

                // Use m_allocator (global) for values so they persist after tl_serializer_walk() returns
                TLString* value = tl_string_create(m_allocator, (char*)token.data.scalar.value);

                TLDEBUG("property %s = %s", tl_string_cstr(property), tl_string_cstr(value))
                // Use allocator (local) for temporary property key used in map lookup
                tl_map_put(m_properties, tl_string_create(m_properties->allocator, tl_string_cstr(property)), value);

                current_key = NULL;  // Reset key after use
            } break;
            // #########################################################################################################
            // YAML_BLOCK_END_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_END_TOKEN: {
                // Pop last segment from path
                if (tl_list_size(path_segments) > 0) {
                    tl_list_pop_back(path_segments);
                }
            } break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    tl_list_destroy(path_segments);
    tl_list_destroy(sequences);
    tl_string_builder_destroy(builder);
    tl_memory_allocator_destroy(allocator);

    TL_PROFILER_POP
}

static void tl_config_print_all() {
    TLINFO("========== YAML PROPERTIES LOADED ==========");
    TLIterator* keys = tl_map_keys(m_properties);
    for (TLString* key = tl_iterator_next(keys); key != NULL; key = tl_iterator_next(keys)) {
        TLList* values = tl_map_get(m_properties, key);
        if (values != NULL && tl_list_size(values) > 0) {
            TLString* value = (TLString*)tl_list_front(values);
            TLINFO("  %s = %s", tl_string_cstr(key), tl_string_cstr(value));
        }
    }
    tl_iterator_destroy(keys);
    TLINFO("============================================");
}