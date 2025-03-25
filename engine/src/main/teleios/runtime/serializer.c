#include <ctype.h>

#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

#include <stdio.h>

#define MAX_PROP 1024
#define TOKEN_VALUE token->data.scalar.value
#define TOKEN_LENGTH token->data.scalar.length
#define IS_BLOCK_EQ(s) tl_char_equals(block, s)
#define IS_PREFIX_EQ(s) tl_char_equals(prefix, s)
#define IS_PROP_EQ(p,b) (IS_PREFIX_EQ(p) && IS_BLOCK_EQ(b))
#define IS_TOKEN_EQ(s) tl_char_equals(TOKEN_VALUE, s)

#define EXPECT(t)                                   \
    yaml_token_delete(&token);                      \
    yaml_parser_scan(&parser, &token);              \
    if (token.type != t) TLFATAL("Unexpect token")

typedef struct {
    const TLString *name;
    u32 sequence;
} TLTuple;

static void tl_serializer_walk(void (*processor)(const char* prefix, const char* block, const yaml_token_t *toke)) {
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
                processor(prefix, block, &token);
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

static b8 tl_serializer_read_engine(const char* prefix, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", prefix, block, token)

    if (!tl_char_start_with(prefix, "engine.")) TLSTACKPOPV(FALSE)

    if (IS_PREFIX_EQ("engine.logging.")) {
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

    if (IS_PREFIX_EQ("engine.graphics.")) {
        if (IS_BLOCK_EQ("vsync")) {
            global->platform.graphics.vsync = tl_char_equals((char*) TOKEN_VALUE, "true");
            TLTRACE("global->platform.graphics.vsync = %d", global->platform.graphics.vsync)
            TLSTACKPOPV(TRUE);
        }
        if (IS_BLOCK_EQ("wireframe")) {
            global->platform.graphics.wireframe = tl_char_equals((char*) TOKEN_VALUE, "true");
            TLTRACE("global->platform.graphics.wireframe = %d", global->platform.graphics.wireframe)
            TLSTACKPOPV(TRUE);
        }
    }

    if (IS_PREFIX_EQ("engine.simulation.")) {
        if (IS_BLOCK_EQ("step")) {
            u8 step = strtol((char*) TOKEN_VALUE, (void*)(TOKEN_VALUE + TOKEN_LENGTH), 10);
            if (step == 0) {
                TLWARN("Failed to read [%s%s] assuming 24", prefix, block);
                step = 24;
            }

            global->application.simulation.step = 1.0f / (f64) step;
            TLTRACE("global->simulation.step = %f", global->application.simulation.step)
            TLSTACKPOPV(TRUE);
        }
    }

    if (IS_PREFIX_EQ("engine.window.")) {
        if (IS_BLOCK_EQ("title")) {
            global->platform.window.title = tl_string_clone(global->platform.arena, (char*) TOKEN_VALUE);
            TLTRACE("global->platform.window.title = %s", TOKEN_VALUE)
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

    TLWARN("Unknown prefix: %s%s", prefix, block);

    TLSTACKPOPV(FALSE);
}

static b8 tl_serializer_read_application(const char* prefix, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", prefix, block, token)

    if (!tl_char_start_with(prefix, "application.")) TLSTACKPOPV(FALSE);
    if (tl_char_start_with(prefix, "application.scenes.")) TLSTACKPOPV(TRUE)

    if (IS_PREFIX_EQ("application.")) {
        if (IS_BLOCK_EQ("version")) {
            global->application.version = tl_string_clone(global->platform.arena, (char*) TOKEN_VALUE);
            TLTRACE("global->application.version = %s", TOKEN_VALUE)
            TLSTACKPOPV(TRUE);
        }
    }

    TLWARN("Unknown prefix: %s%s", prefix, block);
    TLSTACKPOPV(FALSE);
}

static void tl_serializer_read_settings(const char* prefix, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", prefix, block, token)
    if (tl_serializer_read_engine     (prefix, block, token)) TLSTACKPOP
    if (tl_serializer_read_application(prefix, block, token)) TLSTACKPOP
    TLSTACKPOP
}

b8 tl_serializer_read_yaml(void) {
    TLSTACKPUSH
    tl_serializer_walk(tl_serializer_read_settings);
    if (global->platform.window.size.x == 0 || global->platform.window.size.y == 0) {
        TLERROR("Invalid window size")
        TLSTACKPOPV(FALSE)
    }

    if (global->platform.window.title == NULL) {
        TLERROR("Invalid window title")
        TLSTACKPOPV(FALSE)
    }

    TLSTACKPOPV(TRUE)
}

static void tl_serializer_find_scene(const char* prefix, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", prefix, block, token)
    if ( ! IS_PROP_EQ(global->application.scene.prefix, "name") ) TLSTACKPOP

    if (IS_TOKEN_EQ(tl_string(global->application.scene.name))) {
        global->application.scene.found = TRUE;
        TLSTACKPOP
    }

    TLSTACKPOP
}

#define TL_GLPARAM(p, f, g)          \
        if (IS_TOKEN_EQ(f)) {        \
        p = g;                       \
        TLTRACE("%s = GL_%s", #p, f) \
        TLSTACKPOP                   \
    }

static void tl_serializer_load_scene(const char* prefix, const char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", prefix, block, token)

    // Ensure the right [application.scenes.#] is being parsed
    if ( ! tl_char_start_with(prefix, global->application.scene.prefix) ) TLSTACKPOP

    // String used to check if property is the desired key
    char buffer[MAX_PROP];

    // ----------------------------------------------------------------
    //  application.scenes.#.clear_color
    // ----------------------------------------------------------------
    if ( IS_BLOCK_EQ("clear_color") ) {
        const char *current_pos = (const char*) TOKEN_VALUE;
        char *endptr;

        global->application.scene.graphics.clear_color.x = strtof(current_pos, &endptr);
        if (current_pos == endptr) {
            global->application.scene.graphics.clear_color.x = 0.75f;
            global->application.scene.graphics.clear_color.y = 0.23f;
            global->application.scene.graphics.clear_color.z = 0.75f;
            global->application.scene.graphics.clear_color.w = 1.0f;
            TLWARN("Failed to read [scene.clear_color.red] assuming magenta");
            TLSTACKPOP
        }

        current_pos = endptr;
        while (isspace(*current_pos) || *current_pos == ',') {
            current_pos++;
        }

        global->application.scene.graphics.clear_color.y = strtof(current_pos, &endptr);
        if (current_pos == endptr) {
            global->application.scene.graphics.clear_color.x = 0.75f;
            global->application.scene.graphics.clear_color.y = 0.23f;
            global->application.scene.graphics.clear_color.z = 0.75f;
            global->application.scene.graphics.clear_color.w = 1.0f;
            TLWARN("Failed to read [scene.clear_color.green] assuming magenta");
            TLSTACKPOP
        }
        current_pos = endptr;
        while (isspace(*current_pos) || *current_pos == ',') {
            current_pos++;
        }

        global->application.scene.graphics.clear_color.z = strtof(current_pos, &endptr);
        if (current_pos == endptr) {
            global->application.scene.graphics.clear_color.x = 0.75f;
            global->application.scene.graphics.clear_color.y = 0.23f;
            global->application.scene.graphics.clear_color.z = 0.75f;
            global->application.scene.graphics.clear_color.w = 1.0f;
            TLWARN("Failed to read [scene.clear_color.blue] assuming magenta");
            TLSTACKPOP
        }
        current_pos = endptr;
        while (isspace(*current_pos) || *current_pos == ',') {
            current_pos++;
        }

        global->application.scene.graphics.clear_color.w = strtof(current_pos, &endptr);
        if (current_pos == endptr) {
            global->application.scene.graphics.clear_color.x = 0.75f;
            global->application.scene.graphics.clear_color.y = 0.23f;
            global->application.scene.graphics.clear_color.z = 0.75f;
            global->application.scene.graphics.clear_color.w = 1.0f;
            TLWARN("Failed to read [scene.clear_color.alpha] assuming magenta");
            TLSTACKPOP
        }
    }
    // ----------------------------------------------------------------
    //  application.scenes.#.depth
    // ----------------------------------------------------------------
    tl_memory_set(buffer, 0, MAX_PROP);
    tl_char_join(buffer, MAX_PROP, global->application.scene.prefix, "depth.");
    if ( tl_char_equals(buffer, prefix)) {
        if (IS_BLOCK_EQ("enabled") ) {
            global->application.scene.graphics.depth_enabled = TRUE;
            TLTRACE("global->application.scene.graphics.depth_enabled = %d", global->application.scene.graphics.depth_enabled )
            TLSTACKPOP
        }

        if (IS_BLOCK_EQ("function") ) {
            TL_GLPARAM(global->application.scene.graphics.depth_function, "LESS", GL_LESS)
            TL_GLPARAM(global->application.scene.graphics.depth_function, "NEVER", GL_NEVER)
            TL_GLPARAM(global->application.scene.graphics.depth_function, "ALWAYS", GL_ALWAYS)
            TL_GLPARAM(global->application.scene.graphics.depth_function, "LEQUAL", GL_LEQUAL)
            TL_GLPARAM(global->application.scene.graphics.depth_function, "GEQUAL", GL_GEQUAL)
            TL_GLPARAM(global->application.scene.graphics.depth_function, "ALWAYS", GL_ALWAYS)
            TL_GLPARAM(global->application.scene.graphics.depth_function, "GREATER", GL_GREATER)
            TL_GLPARAM(global->application.scene.graphics.depth_function, "NOTEQUAL", GL_NOTEQUAL)
        }
    }
    // ----------------------------------------------------------------
    //  application.scenes.#.blend
    // ----------------------------------------------------------------
    tl_memory_set(buffer, 0, MAX_PROP);
    tl_char_join(buffer, MAX_PROP, global->application.scene.prefix, "blend.");
    if (tl_char_start_with(prefix, buffer)) {
        if (IS_BLOCK_EQ("enabled") ) {
            global->application.scene.graphics.blend_enabled = TRUE;
            TLTRACE("global->application.scene.graphics.blend_enabled = %d", global->application.scene.graphics.blend_enabled )
            TLSTACKPOP
        }

        if (IS_BLOCK_EQ("equation") ) {
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "MIN", GL_MIN)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "MAX", GL_MAX)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_ADD", GL_FUNC_ADD)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_SUBTRACT", GL_FUNC_SUBTRACT)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_REVERSE_SUBTRACT", GL_FUNC_REVERSE_SUBTRACT)
        }

        tl_memory_set(buffer, 0, MAX_PROP);
        tl_char_join(buffer, MAX_PROP, global->application.scene.prefix, "blend.function.");
        if (tl_char_equals(prefix, buffer)) {
            if (IS_BLOCK_EQ("source") ) {
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ZERO", GL_ZERO)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE", GL_ONE)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "SRC_COLOR", GL_SRC_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "DST_COLOR", GL_DST_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "SRC_ALPHA", GL_SRC_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "DST_ALPHA", GL_DST_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "CONSTANT_COLOR", GL_CONSTANT_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_CONSTANT_COLOR", GL_ONE_MINUS_CONSTANT_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "CONSTANT_ALPHA", GL_CONSTANT_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_CONSTANT_ALPHA", GL_ONE_MINUS_CONSTANT_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "SRC1_COLOR", GL_SRC1_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_SRC1_COLOR", GL_ONE_MINUS_SRC1_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "SRC1_ALPHA", GL_SRC1_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_src, "ONE_MINUS_SRC1_ALPHA", GL_ONE_MINUS_SRC1_ALPHA)
            }

            if (IS_BLOCK_EQ("target") ) {
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ZERO", GL_ZERO)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE", GL_ONE)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "SRC_COLOR", GL_SRC_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "DST_COLOR", GL_DST_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "SRC_ALPHA", GL_SRC_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "DST_ALPHA", GL_DST_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "CONSTANT_COLOR", GL_CONSTANT_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_CONSTANT_COLOR", GL_ONE_MINUS_CONSTANT_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "CONSTANT_ALPHA", GL_CONSTANT_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_CONSTANT_ALPHA", GL_ONE_MINUS_CONSTANT_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "SRC1_COLOR", GL_SRC1_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_SRC1_COLOR", GL_ONE_MINUS_SRC1_COLOR)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "SRC1_ALPHA", GL_SRC1_ALPHA)
                TL_GLPARAM(global->application.scene.graphics.blend_function_tgt, "ONE_MINUS_SRC1_ALPHA", GL_ONE_MINUS_SRC1_ALPHA)
            }
        }
    }
    // ----------------------------------------------------------------
    //  application.scenes.#.camera
    // ----------------------------------------------------------------

    // ----------------------------------------------------------------
    //  application.scenes.#.actors
    // ----------------------------------------------------------------
    TLSTACKPOP
}

b8 tl_serializer_read_scene(const char *name) {
    TLSTACKPUSHA("%s", name)
    tl_memory_arena_reset(global->application.scene.arena);
    global->application.scene.name = tl_string_clone(global->application.scene.arena, name);

    for (u8 sequence = 0 ; sequence < U8_MAX ; sequence++) {
        tl_memory_set(global->application.scene.prefix, 0, U8_MAX);
        snprintf(global->application.scene.prefix, U8_MAX, "application.scenes.%d.", sequence);

        global->application.scene.found = FALSE;
        tl_serializer_walk(tl_serializer_find_scene);
        if (global->application.scene.found) {
            TLDEBUG("Found scene [%s] at position [%u]", name, sequence)
            tl_serializer_walk(tl_serializer_load_scene);

            TLSTACKPOPV(TRUE)
        }
    }

    TLSTACKPOPV(FALSE)
}
