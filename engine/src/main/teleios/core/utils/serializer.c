#include "teleios/globals.h"
#include "teleios/defines.h"
#include "teleios/core.h"

#include <ctype.h>
#include <stdio.h>

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) TLFATAL("Unexpect token")

typedef struct {
    const TLString *name;
    u32 sequence;
} TLTuple;

void tl_serializer_walk(void (*processor)(const char* prefix, const char* block, const char *value, u64 length)) {
    TLSTACKPUSHA("%s", tl_string(global->yaml))
    FILE* file = fopen(tl_string(global->yaml), "r");
    if (file == NULL) TLFATAL("Failed to open %s", tl_string(global->yaml));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); TLFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    u8 block_index = 0;
    u16 prefix_index = 0;
    yaml_token_t token;
    TLMemoryArena *arena = tl_memory_arena_create(TLKIBIBYTES(4));
    TLList *sequences = tl_list_create(arena);
    char block[U8_MAX]; // current YAML_KEY_TOKEN
    char prefix[U16_MAX]; // YAML_KEY_TOKEN path to YAML_SCALAR_TOKEN value

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
                TLTuple *tuple = tl_memory_alloc(arena, sizeof(TLTuple), TL_MEMORY_SERIALIZER);
                tuple->sequence = 0;

                tuple->name = tl_string_clone(arena, prefix);
                tl_string_join(tuple->name, block);
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
                    tl_memory_copy(prefix + prefix_index, block, block_index);
                    prefix_index += block_index;

                    tl_memory_copy(prefix + prefix_index, ".", 1);
                    prefix_index++;
                }
                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                tl_memory_copy(block, token.data.scalar.value, token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(block + token.data.scalar.length, 0, block_index - token.data.scalar.length);
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
                    tl_memory_copy(prefix + prefix_index, block, block_index);
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
                TLIterator *iterator = tl_list_iterator_create(sequences);
                for (TLTuple *tuple = tl_list_iterator_next(iterator) ; tuple != NULL; tuple = tl_list_iterator_next(iterator)) {
                    if (tl_string_equals(tuple->name, prefix)) {
                        string = tl_string_from_i32(arena, (i32) tuple->sequence, 10);
                        tuple->sequence++;
                    }
                }

                u32 length = tl_string_length(string);
                tl_memory_copy(block, (void*) tl_string(string), length);

                if (block_index > length) {
                    tl_memory_set(block + length, 0, block_index - length);
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
                tl_memory_copy(block, token.data.scalar.value, token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(block + token.data.scalar.length, 0, block_index - token.data.scalar.length);
                }

                block_index = token.data.scalar.length;
            } break;
            // #########################################################################################################
            // YAML_SCALAR_TOKEN
            // #########################################################################################################
            case YAML_SCALAR_TOKEN: {
                processor(prefix, block, (const char*)token.data.scalar.value, token.data.scalar.length);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_END_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_END_TOKEN: {
                b8 found = FALSE;
                for (u16 i = prefix_index - 2 ; i > 0 ; --i) {
                    if (*(prefix + i)  == '.') {
                        tl_memory_set(prefix + i + 1, 0, U16_MAX - i);
                        prefix_index = i + 1;
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    tl_memory_set(prefix, 0, U16_MAX);
                    prefix_index = 0;
                }

                tl_memory_set(block, 0, U8_MAX - block_index);
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

    TLSTACKPOP
}
