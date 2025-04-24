#include <yaml.h>
#include "teleios/teleios.h"

#ifndef TL_YAML_PROPERTY_MAX_SIZE
#define TL_YAML_PROPERTY_MAX_SIZE 1024
#endif

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) KFATAL("Unexpect token")

typedef struct {
    const KString *name;
    u32 sequence;
} TLTuple;

void tl_yaml_walk(const char *yaml, KCollection *properties) {
    K_FRAME_PUSH_WITH("0x%p")
    FILE* file = fopen(yaml, "r");
    if (file == NULL) KFATAL("Failed to open %s", yaml);

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); KFATAL("Failed to initialize parser!"); }

    k_memory_set(parser.buffer.start, 0, 16384 * 3);
    k_memory_set(parser.raw_buffer.start, 0, 16384);

    yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);
    yaml_parser_set_input_file(&parser, file);

    u8 element_index = 0;
    u16 prefix_index = 0;
    yaml_token_t token;
    KAllocator *allocator = k_memory_allocator_create(K_MEMORY_ALLOCATOR_LINEAR, K_KIBI_BYTES(4));
    KCollection *sequences = k_list_create(allocator);
    char element[U8_MAX]; // current YAML_KEY_TOKEN
    char prefix[U16_MAX]; // YAML_KEY_TOKEN path to YAML_SCALAR_TOKEN value
    char property[TL_YAML_PROPERTY_MAX_SIZE];

    do {
        yaml_parser_scan(&parser, &token);
        switch (parser.error) {
            case YAML_EMITTER_ERROR : KFATAL("%s: %s", "Cannot emit a YAML stream", parser.problem)
            case YAML_SCANNER_ERROR : KFATAL("%s: %s", "Cannot scan the input stream", parser.problem)
            case YAML_PARSER_ERROR  : KFATAL("%s: %s", "Cannot parse the input stream", parser.problem)
            case YAML_COMPOSER_ERROR: KFATAL("%s: %s", "Cannot compose a YAML document", parser.problem)
            case YAML_WRITER_ERROR  : KFATAL("%s: %s", "Cannot write to the output stream", parser.problem)
            case YAML_READER_ERROR  : KFATAL("%s: %s", "Cannot read or decode the input stream", parser.problem)
            case YAML_MEMORY_ERROR  : KFATAL("%s: %s", "Cannot allocate or reallocate a block of memory", parser.problem)
        }

        switch(token.type) {
            default: continue;
            // #########################################################################################################
            // YAML_BLOCK_SEQUENCE_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_SEQUENCE_START_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                TLTuple *tuple = k_memory_allocator_alloc(allocator, sizeof(TLTuple), TL_MEMORY_YAML);
                tuple->sequence = 0;

                tuple->name = k_string_clone(allocator, prefix);
                k_string_join(tuple->name, element);
                k_string_join(tuple->name, ".");

                k_list_add(sequences, tuple);
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
                if (element_index != 0) {
                    k_memory_copy(prefix + prefix_index, element, element_index);
                    prefix_index += element_index;

                    k_memory_copy(prefix + prefix_index, ".", 1);
                    prefix_index++;
                }
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                k_memory_copy(element, token.data.scalar.value, token.data.scalar.length);
                if (element_index > token.data.scalar.length) {
                    k_memory_set(element + token.data.scalar.length, 0, element_index - token.data.scalar.length);
                }
                element_index = token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_BLOCK_ENTRY_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_ENTRY_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                if (element_index != 0) {
                    k_memory_copy(prefix + prefix_index, element, element_index);
                    prefix_index += element_index;

                    k_memory_copy(prefix + prefix_index, ".", 1);
                    prefix_index++;
                }
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                KString *string = NULL;

                // -----------------------------------------------------------
                // Try tro increase the counter
                // -----------------------------------------------------------
                KIterator *iterator = k_iterator_create(NULL, sequences);
                for (TLTuple *tuple = k_iterator_next(iterator) ; tuple != NULL; tuple = k_iterator_next(iterator)) {
                    if (k_string_equals(tuple->name, prefix)) {
                        string = k_string_from_i32(allocator, (i32) tuple->sequence, 10);
                        tuple->sequence++;
                    }
                }

                u32 length = k_string_length(string);
                k_memory_copy(element, (void*) k_string(string), length);

                if (element_index > length) {
                    k_memory_set(element + length, 0, element_index - length);
                }

                element_index = k_string_length(string);
            } continue;
            // #########################################################################################################
            // YAML_KEY_TOKEN
            // #########################################################################################################
            case YAML_KEY_TOKEN: {
                EXPECT(YAML_SCALAR_TOKEN)
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                k_memory_copy(element, token.data.scalar.value, token.data.scalar.length);
                if (element_index > token.data.scalar.length) {
                    k_memory_set(element + token.data.scalar.length, 0, element_index - token.data.scalar.length);
                }

                element_index = token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_SCALAR_TOKEN
            // #########################################################################################################
            case YAML_SCALAR_TOKEN: {
                k_memory_set(property, 0, TL_YAML_PROPERTY_MAX_SIZE);
                k_char_join(property, TL_YAML_PROPERTY_MAX_SIZE, prefix, element);

                TLRuntime *runtime = tl_runtime_get();
                KString *value =  k_string_clone(runtime->allocator, (char*) token.data.scalar.value);
                k_map_put(properties, property, value);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_END_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_END_TOKEN: {
                b8 found = false;
                for (u16 i = prefix_index - 2 ; i > 0 ; --i) {
                    if (*(prefix + i)  == '.') {
                        k_memory_set(prefix + i + 1, 0, U16_MAX - i);
                        prefix_index = i + 1;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    k_memory_set(prefix, 0, U16_MAX);
                    prefix_index = 0;
                }

                k_memory_set(element, 0, U8_MAX - element_index);
                element_index = 0;
            } break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    k_memory_allocator_destroy(allocator);

    K_FRAME_POP
}