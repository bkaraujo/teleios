#include "container_types.inl"
#include "teleios/teleios.h"

static TLAllocator* m_allocator;
static TLMap* m_properties;

static void tl_serializer_walk(TLString* file_path);

b8 tl_config_initialize(TLString* file_path) {
    TL_PROFILER_PUSH_WITH("0x%p", file_path)

    m_allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    m_properties = tl_map_create(m_allocator, 32);

    tl_serializer_walk(file_path);

    TL_PROFILER_POP_WITH(true)
}

void* tl_config_get(TLString* property) {
    TL_PROFILER_PUSH_WITH("0x%p", property)
    if (property == NULL) {
        TLWARN("Attempting to get a NULL property");
        TL_PROFILER_POP_WITH(NULL)
    }
    TLList* list = tl_map_get(m_properties, property);
    void* value = tl_list_front(list);

    TL_PROFILER_POP_WITH(value)
}

TLList* tl_config_list(TLString* property) {
    TL_PROFILER_PUSH_WITH("0x%p", property)
    if (property == NULL) {
        TLWARN("Attempting to list a NULL property");
        TL_PROFILER_POP_WITH(NULL)
    }
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

static void tl_serializer_walk(TLString* file_path) {
    TL_PROFILER_PUSH_WITH("0x%p", file_path)
    FILE* file = fopen(tl_string_cstr(file_path), "r");
    if (file == NULL) TLFATAL("Failed to open %s", tl_string_cstr(file_path));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); TLFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    u8 block_index = 0;
    u16 prefix_index = 0;
    yaml_token_t token;
    TLAllocator *allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);
    TLList *sequences = tl_list_create(allocator);
    char element[U8_MAX]; // current YAML_KEY_TOKEN
    char prefix[U16_MAX]; // YAML_KEY_TOKEN path to YAML_SCALAR_TOKEN value
    char property[TL_YAML_PROPERTY_MAX_SIZE];

    do {
        yaml_parser_scan(&parser, &token);

        switch(token.type) {
            default: continue;
            case YAML_NO_TOKEN: {
                TLFATAL("Malformed YAML token");
            } break;
            // #########################################################################################################
            // YAML_BLOCK_SEQUENCE_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_SEQUENCE_START_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                TLTuple *tuple = tl_memory_alloc(allocator, sizeof(TLTuple), TL_MEMORY_SERIALIZER);
                tuple->sequence = 0;

                tuple->name = tl_string_create(allocator, prefix);
                tl_string_append(tuple->name, element);
                tl_string_append(tuple->name, ".");

                tl_list_push_back(sequences, tuple);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_MAPPING_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_MAPPING_START_TOKEN: {
                EXPECT(YAML_KEY_TOKEN)
                EXPECT(YAML_SCALAR_TOKEN)
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                if (block_index != 0) {
                    tl_memory_copy(prefix + prefix_index, element, block_index);
                    prefix_index += block_index;

                    tl_memory_copy(prefix + prefix_index, ".", 1);
                    prefix_index++;
                }
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                tl_memory_copy(element, token.data.scalar.value, (u32)token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(element + token.data.scalar.length, 0, (u32)(block_index - token.data.scalar.length));
                }
                block_index = (u8)token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_BLOCK_ENTRY_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_ENTRY_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                if (block_index != 0) {
                    tl_memory_copy(prefix + prefix_index, element, block_index);
                    prefix_index += block_index;

                    tl_memory_copy(prefix + prefix_index, ".", 1);
                    prefix_index++;
                }
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                TLString *string = NULL;

                // -----------------------------------------------------------
                // Try tro increase the counter
                // -----------------------------------------------------------
                TLIterator *iterator = tl_list_iterator(sequences);
                for (TLTuple *tuple = tl_iterator_next(iterator) ; tuple != NULL; tuple = tl_iterator_next(iterator)) {
                    if (tl_string_equals_cstr(tuple->name, prefix)) {
                        string = tl_number_i32_to_char(allocator, (i32) tuple->sequence, 10);
                        tuple->sequence++;
                    }
                }

                u32 length = tl_string_length(string);
                tl_memory_copy(element, (void*) tl_string_cstr(string), length);

                if (block_index > length) {
                    tl_memory_set(element + length, 0, block_index - length);
                }

                block_index = (u8)tl_string_length(string);
            } continue;
            // #########################################################################################################
            // YAML_KEY_TOKEN
            // #########################################################################################################
            case YAML_KEY_TOKEN: {
                EXPECT(YAML_SCALAR_TOKEN)
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                tl_memory_copy(element, token.data.scalar.value, (u32)token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(element + token.data.scalar.length, 0, (u32)(block_index - token.data.scalar.length));
                }

                block_index = (u8)token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_SCALAR_TOKEN
            // #########################################################################################################
            case YAML_SCALAR_TOKEN: {
                tl_memory_set(property, 0, TL_YAML_PROPERTY_MAX_SIZE);
                tl_cstr_join(property, TL_YAML_PROPERTY_MAX_SIZE, prefix, element);
                tl_map_put(m_properties, tl_string_create(m_properties->allocator, property), tl_string_create(allocator, (char*) token.data.scalar.value));
            } break;
            // #########################################################################################################
            // YAML_BLOCK_END_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_END_TOKEN: {
                b8 found = false;
                for (u16 i = prefix_index - 2 ; i > 0 ; --i) {
                    if (*(prefix + i)  == '.') {
                        tl_memory_set(prefix + i + 1, 0, U16_MAX - i);
                        prefix_index = i + 1;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    tl_memory_set(prefix, 0, U16_MAX);
                    prefix_index = 0;
                }

                tl_memory_set(element, 0, U8_MAX - block_index);
                block_index = 0;
            } break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    tl_memory_allocator_destroy(allocator);

    TL_PROFILER_POP
}