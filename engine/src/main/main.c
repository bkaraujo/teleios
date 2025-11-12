#include "teleios/teleios.h"
#include "teleios/application.h"

static void tl_serializer_walk(TLMap *properties);

int main(const int argc, const char* argv[]) {
    (void)argc;  // Unused parameter
    (void)argv;  // Unused parameter
    tl_logger_set_level(TL_LOG_LEVEL_VERBOSE);
    // ####################################################################
    // 1 - Iniciar OS
    // ####################################################################
    if (!tl_platform_initialize()) TLFATAL("Platform failed to initialize")
    // ####################################################################
    // 2 - Carregar DLL Cliente
    // ####################################################################

    // ####################################################################
    // 3 - Iniciar Engine
    // ####################################################################
    if (!tl_application_initialize()) {
        if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")
        TLFATAL("Application failed to initialize")
    }
    // ####################################################################
    // 4 - Executar aplicação
    // ####################################################################
    if (!tl_application_run()) {
        if (!tl_application_terminate()) TLERROR("Platform failed to terminate")
        if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")
        TLFATAL("Application failed to run")
    }
    
    if (!tl_application_terminate()) TLERROR("Application failed to terminate")

    if (!tl_platform_terminate()) TLERROR("Platform failed to terminate")

    return 0;
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

static void tl_serializer_walk(TLMap *properties) {
    TL_PROFILER_PUSH_WITH("0x%p")
    FILE* file = fopen(tl_string(global->yaml), "r");
    if (file == NULL) TLFATAL("Failed to open %s", tl_string(global->yaml));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); TLFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    u8 block_index = 0;
    u16 prefix_index = 0;
    yaml_token_t token;
    TLAllocator *allocator = tl_memory_allocator_create(TL_ALLOCATOR_LINEAR, TL_KIBI_BYTES(4));
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
                tl_memory_copy(element, token.data.scalar.value, token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(element + token.data.scalar.length, 0, block_index - token.data.scalar.length);
                }
                block_index = token.data.scalar.length;
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

                block_index = tl_string_length(string);
            } continue;
            // #########################################################################################################
            // YAML_KEY_TOKEN
            // #########################################################################################################
            case YAML_KEY_TOKEN: {
                EXPECT(YAML_SCALAR_TOKEN)
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                tl_memory_copy(element, token.data.scalar.value, token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(element + token.data.scalar.length, 0, block_index - token.data.scalar.length);
                }

                block_index = token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_SCALAR_TOKEN
            // #########################################################################################################
            case YAML_SCALAR_TOKEN: {
                tl_memory_set(property, 0, TL_YAML_PROPERTY_MAX_SIZE);
                tl_cstr_join(property, TL_YAML_PROPERTY_MAX_SIZE, prefix, element);
                tl_map_put(properties, property, tl_string_create(allocator, (char*) token.data.scalar.value));
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