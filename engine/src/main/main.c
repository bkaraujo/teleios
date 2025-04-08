#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"
#include "teleios/application.h"
    
TLGlobal *global;

static void tl_serializer_walk(TLMap *properties);

int main (const int argc, const char *argv[]) {
    if (argc != 2) {
        BKSERROR("argc != 2")
        BKS_TRACE_POPV(99)
    }

    bks_logger_loglevel(BKS_LOG_LEVEL_VERBOSE);

    BKSINFO("Initializing %s", argv[1]);

    TLGlobal local = {0};
    global = &local;
    bks_memory_set(global, 0, sizeof(TLGlobal));

    BKS_TRACE_PUSHA("%i, 0%xp", argc, argv)

    if (!tl_platform_initialize()) {
        BKSERROR("Platform failed to initialize")
        if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_memory_initialize()) {
        BKSERROR("Memory failed to initialize")
        if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    global->application.running = true;
    global->arena = tl_memory_arena_create(BKS_MEBI_BYTES(10));

    global->yaml = tl_string_clone(global->arena, argv[1]);
    global->rootfs = tl_filesystem_get_parent(global->yaml);
    global->properties = tl_map_create(global->arena);

    tl_serializer_walk(global->properties);

    if (!tl_runtime_initialize()) {
        BKSERROR("Runtime failed to initialize")
        if (!tl_runtime_terminate ()) BKSERROR("Runtime failed to terminate")
        if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_load()) {
        BKSERROR("Application failed to initialize");
        if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_initialize()) {
        BKSERROR("Engine failed to initialize");
        if (!tl_application_terminate()) BKSERROR("Application failed to terminate")
        if (!tl_runtime_terminate    ()) BKSERROR("Runtime failed to terminate")
        if (!tl_platform_terminate   ()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_run()) {
        BKSERROR("Application failed to execute")
        if (!tl_application_terminate()) BKSERROR("Application failed to terminate")
        if (!tl_runtime_terminate    ()) BKSERROR("Runtime failed to terminate")
        if (!tl_platform_terminate   ()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_application_terminate()) {
        BKSERROR("Application failed to terminate")
        if (!tl_runtime_terminate ()) BKSERROR("Runtime failed to terminate")
        if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_runtime_terminate()) {
        BKSERROR("Runtime failed to terminate")
        if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    tl_memory_arena_destroy(global->arena);

    if (!tl_memory_terminate()) {
        BKSERROR("Memory failed to terminate")
        if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")
        exit(99);
    }

    if (!tl_platform_terminate()) BKSFATAL("Platform failed to terminate")

    BKSINFO("Exiting")
    BKS_TRACE_POPV(0)
}

// #####################################################################################################################
//
//                                                     SERIALIZER
//
// #####################################################################################################################
#include <ctype.h>
#include <stdio.h>

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) BKSFATAL("Unexpect token")

typedef struct {
    const TLString *name;
    u32 sequence;
} TLTuple;

static void tl_serializer_walk(TLMap *properties) {
    BKS_TRACE_PUSHA("0x%p")
    FILE* file = fopen(tl_string(global->yaml), "r");
    if (file == NULL) BKSFATAL("Failed to open %s", tl_string(global->yaml));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); BKSFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    u8 block_index = 0;
    u16 prefix_index = 0;
    yaml_token_t token;
    TLMemoryArena *arena = tl_memory_arena_create(BKS_KIBI_BYTES(4));
    TLList *sequences = tl_list_create(arena);
    char element[U8_MAX]; // current YAML_KEY_TOKEN
    char prefix[U16_MAX]; // YAML_KEY_TOKEN path to YAML_SCALAR_TOKEN value
    char property[TL_YAML_PROPERTY_MAX_SIZE];

    do {
        yaml_parser_scan(&parser, &token);

        switch(token.type) {
            default: continue;
            case YAML_NO_TOKEN: {
                BKSFATAL("Malformed YAML token");
            } break;
            // #########################################################################################################
            // YAML_BLOCK_SEQUENCE_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_SEQUENCE_START_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'prefix'
                // -----------------------------------------------------------
                TLTuple *tuple = tl_memory_alloc(arena, sizeof(TLTuple), TL_MEMORY_SERIALIZER);
                tuple->sequence = 0;

                tuple->name = tl_string_clone(arena, prefix);
                tl_string_join(tuple->name, element);
                tl_string_join(tuple->name, ".");

                tl_list_add(sequences, tuple);
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
                TLIterator *iterator = tl_list_iterator_create(NULL, sequences);
                for (TLTuple *tuple = tl_iterator_next(iterator) ; tuple != NULL; tuple = tl_iterator_next(iterator)) {
                    if (tl_string_equals(tuple->name, prefix)) {
                        string = tl_string_from_i32(arena, (i32) tuple->sequence, 10);
                        tuple->sequence++;
                    }
                }

                u32 length = tl_string_length(string);
                tl_memory_copy(element, (void*) tl_string(string), length);

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
                bks_memory_set(property, 0, TL_YAML_PROPERTY_MAX_SIZE);
                tl_char_join(property, TL_YAML_PROPERTY_MAX_SIZE, prefix, element);
                tl_map_put(properties, property, tl_string_clone(global->arena, (char*) token.data.scalar.value));
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
    tl_memory_arena_destroy(arena);

    BKS_TRACE_POP
}