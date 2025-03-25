#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

#include <stdio.h>

#define IS_BLOCK_EQ(s) tl_char_equals(block, s)
#define IS_PROP_EQ(s) tl_char_equals(property, s)
#define IS_TOKEN_EQ(s) tl_char_equals(token->data.scalar.value, s)

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) TLFATAL("Unexpect token")

typedef struct {
    const TLString *name;
    u32 sequence;
} TLTuple;

static void tl_serializer_walk(void (*processor)(const char* property, const char* block, const yaml_token_t *toke)) {
    TLSTACKPUSHA("%s", global->yaml)
    TLDEBUG("Loading %s", global->yaml)
    FILE* file = fopen(global->yaml, "r");
    if (file == NULL) TLFATAL("Failed to open %s", global->yaml);

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) { fclose(file); TLFATAL("Failed to initialize parser!"); }
    yaml_parser_set_input_file(&parser, file);

    u8 block_index = 0;
    u16 property_index = 0;
    yaml_token_t token;
    TLMemoryArena *arena = tl_memory_arena_create(TLKIBIBYTES(4));
    TLList *sequences = tl_list_create(arena);
    char block[U8_MAX]; // current YAML_KEY_TOKEN
    char property[U16_MAX]; // YAML_KEY_TOKEN path to YAML_SCALAR_TOKEN value

    do {
        yaml_parser_scan(&parser, &token);

        switch(token.type) {
            default: continue;
            // #########################################################################################################
            // YAML_BLOCK_SEQUENCE_START_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_SEQUENCE_START_TOKEN: {
                // -----------------------------------------------------------
                // If it's already inside a block persist into 'property'
                // -----------------------------------------------------------
                TLTuple *tuple = tl_memory_alloc(arena, sizeof(TLTuple), TL_MEMORY_SERIALIZER);
                tuple->sequence = 0;

                tuple->name = tl_string_clone(arena, property);
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
                // If it's already inside a block persist into 'property'
                // -----------------------------------------------------------
                if (block_index != 0) {
                    tl_memory_copy(property + property_index, block, block_index);
                    property_index += block_index;

                    tl_memory_copy(property + property_index, ".", 1);
                    property_index++;
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
                // If it's already inside a block persist into 'property'
                // -----------------------------------------------------------
                if (block_index != 0) {
                    tl_memory_copy(property + property_index, block, block_index);
                    property_index += block_index;

                    tl_memory_copy(property + property_index, ".", 1);
                    property_index++;
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
                    if (tl_string_equals(tuple->name, property)) {
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
                processor(property, block, &token);
            } break;
            // #########################################################################################################
            // YAML_BLOCK_END_TOKEN
            // #########################################################################################################
            case YAML_BLOCK_END_TOKEN: {
                b8 found = FALSE;
                for (u16 i = property_index - 2 ; i > 0 ; --i) {
                    if (*(property + i)  == '.') {
                        tl_memory_set(property + i + 1, 0, U16_MAX - i);
                        property_index = i + 1;
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    tl_memory_set(property, 0, U16_MAX);
                    property_index = 0;
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

static b8 tl_serializer_read_engine(const char* property, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", property, block, token)

    if (!tl_char_start_with(property, "engine.")) TLSTACKPOPV(FALSE)

    if (IS_PROP_EQ("engine.logging.")) {
        if (IS_BLOCK_EQ("level")) {
            if (IS_TOKEN_EQ("verbose")) { tl_logger_loglevel(TL_LOG_LEVEL_VERBOSE); TLSTACKPOPV(TRUE); }
            if (IS_TOKEN_EQ(  "trace")) { tl_logger_loglevel(  TL_LOG_LEVEL_TRACE); TLSTACKPOPV(TRUE); }
            if (IS_TOKEN_EQ(  "debug")) { tl_logger_loglevel(  TL_LOG_LEVEL_DEBUG); TLSTACKPOPV(TRUE); }
            if (IS_TOKEN_EQ(   "info")) { tl_logger_loglevel(   TL_LOG_LEVEL_INFO); TLSTACKPOPV(TRUE); }
            if (IS_TOKEN_EQ(   "warn")) { tl_logger_loglevel(   TL_LOG_LEVEL_WARN); TLSTACKPOPV(TRUE); }
            if (IS_TOKEN_EQ(  "error")) { tl_logger_loglevel(  TL_LOG_LEVEL_ERROR); TLSTACKPOPV(TRUE); }
            if (IS_TOKEN_EQ(  "fatal")) { tl_logger_loglevel(  TL_LOG_LEVEL_FATAL); TLSTACKPOPV(TRUE); }
        }
    }

    if (IS_PROP_EQ("engine.graphics.")) {
        if (IS_BLOCK_EQ("vsync")) {
            global->platform.graphics.vsync = tl_char_equals((char*) token->data.scalar.value, "true");
            TLTRACE("global->platform.graphics.vsync = %d", global->platform.graphics.vsync)
            TLSTACKPOPV(TRUE);
        }
        if (IS_BLOCK_EQ("wireframe")) {
            global->platform.graphics.wireframe = tl_char_equals((char*) token->data.scalar.value, "true");
            TLTRACE("global->platform.graphics.wireframe = %d", global->platform.graphics.wireframe)
            TLSTACKPOPV(TRUE);
        }
    }

    if (IS_PROP_EQ("engine.simulation.")) {
        if (IS_BLOCK_EQ("step")) {
            u8 step = strtol((char*) token->data.scalar.value, (void*)(token->data.scalar.value + token->data.scalar.length), 10);
            if (step == 0) {
                TLWARN("Failed to read [%s%s] assuming 24", property, block);
                step = 24;
            }

            global->application.simulation.step = 1.0f / (f64) step;
            TLTRACE("global->simulation.step = %f", global->application.simulation.step)
            TLSTACKPOPV(TRUE);
        }
    }

    TLWARN("Unknown property: %s%s", property, block);

    TLSTACKPOPV(FALSE);
}

static b8 tl_serializer_read_application(const char* property, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", property, block, token)

    if (!tl_char_start_with(property, "application.")) TLSTACKPOPV(FALSE);
    if (tl_char_start_with(property, "application.scenes.")) TLSTACKPOPV(TRUE)

    if (IS_PROP_EQ("application.")) {
        if (IS_BLOCK_EQ("version")) {
            global->application.version = tl_string_clone(global->platform.arena, (char*) token->data.scalar.value);
            TLTRACE("global->application.version = %s", token->data.scalar.value)
            TLSTACKPOPV(TRUE);
        }
    }

    if (IS_PROP_EQ("application.window.")) {
        if (IS_BLOCK_EQ("title")) {
            global->platform.window.title = tl_string_clone(global->platform.arena, (char*) token->data.scalar.value);
            TLTRACE("global->platform.window.title = %s", token->data.scalar.value)
            TLSTACKPOPV(TRUE);
        }

        if (IS_BLOCK_EQ("size")) {
            if (IS_TOKEN_EQ( "SD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_SD; }
            if (IS_TOKEN_EQ( "HD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_HD; }
            if (IS_TOKEN_EQ("FHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_FHD; }
            if (IS_TOKEN_EQ("QHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_QHD; }
            if (IS_TOKEN_EQ("UHD")) { global->platform.window.size.x = TL_VIDEO_RESOLUTION_UHD; }

            global->platform.window.size.y = (global->platform.window.size.x * 9) / 16;
            TLTRACE("global->platform.window.size = %u x %u", global->platform.window.size.x, global->platform.window.size.y)
            TLSTACKPOPV(TRUE);
        }
    }

    TLWARN("Unknown property: %s%s", property, block);
    TLSTACKPOPV(FALSE);
}

static void tl_serializer_read_setup(const char* property, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", property, block, token)
    if (tl_serializer_read_engine     (property, block, token)) TLSTACKPOP
    if (tl_serializer_read_application(property, block, token)) TLSTACKPOP
    TLSTACKPOP
}

void tl_serializer_read(void) {
    TLSTACKPUSH
    tl_serializer_walk(tl_serializer_read_setup);
    TLSTACKPOP
}

static void tl_serializer_read_scene_with_id(const char *id, const char* property, const char* block, const yaml_token_t *token) {

}

static void tl_serializer_read_scene(const char* property, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", property, block, token)
    if (!tl_char_start_with(property, "application.scenes.")) TLSTACKPOP

    const u64 idx = tl_char_index_of(property + 19, '.');
    char sequence[idx + 1];
    tl_memory_set(sequence, 0 , idx + 1);
    tl_memory_copy(sequence, (void*)property + 19, idx);



    TLWARN("Processing property: %s%s", property, block)
    TLSTACKPOP
}

b8 tl_serializer_scene(const char *name) {
    TLSTACKPUSHA("%s", name)
    tl_memory_arena_reset(global->application.scene.arena);
    global->application.scene.name = tl_string_clone(global->application.scene.arena, name);

    tl_serializer_walk(tl_serializer_read_scene);
    TLSTACKPOPV(FALSE)
}
