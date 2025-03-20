#include "teleios/core.h"
#include "teleios/core/serializer.h"
#include <stdio.h>

#define IS_BLOCK_EQ(s) tl_char_equals(block, s)
#define IS_PROP_EQ(s) tl_char_equals(property, s)
#define IS_TOKEN_EQ(s) tl_char_equals(token->data.scalar.value, s)

static b8 tl_serializer_read_engine(char* property, char* block, const yaml_token_t *token);
static b8 tl_serializer_read_application(char* property, char* block, const yaml_token_t *token);

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
static void itoa(int value, char* result, const int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return; }

    char* ptr = result, *ptr1 = result;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';

    // Reverse the string
    while(ptr1 < ptr) {
        const char tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
}

TLINLINE static void tl_serializer_block_fill(char* block, u8 block_index, char* buffer) {
    tl_memory_copy(block, buffer, sizeof(buffer));
    if (block_index > sizeof(buffer)) {
        tl_memory_set(block + sizeof(buffer), 0, block_index - sizeof(buffer));
    }
}

void tl_serializer_read(const char *file_name) {
    TLSTACKPUSHA("%s", file_name)

    TLINFO("Loading %s", file_name)
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        TLFATAL("Failed to open %s", file_name);
    }

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        fclose(file);
        TLFATAL("Failed to initialize parser!");
    }

    yaml_parser_set_input_file(&parser, file);

    u16 block_sequence = 0;

    u8 block_index;
    char block[U8_MAX]; // current YAML_KEY_TOKEN

    u16 property_index;
    char property[U16_MAX]; // YAML_KEY_TOKEN path to YAML_SCALAR_TOKEN value
    
    yaml_token_t token;

    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type) {
            default: continue;
            case YAML_BLOCK_MAPPING_START_TOKEN: {
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);
                if (token.type != YAML_KEY_TOKEN) TLFATAL("Expected YAML_KEY_TOKEN")

                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);
                if (token.type != YAML_SCALAR_TOKEN) TLFATAL("Expected YAML_SCALAR_TOKEN")
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

            case YAML_BLOCK_SEQUENCE_START_TOKEN: {
                block_sequence = 0;
            } break;

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
                if (block_sequence <= 9) {
                    char buffer[1]; itoa(block_sequence, buffer, 10);
                    tl_serializer_block_fill(block, block_index, buffer);
                    block_sequence++;
                    block_index = sizeof(buffer);
                    continue;
                }

                if (block_sequence > 10) {
                    char buffer[2]; itoa(block_sequence, buffer, 10);
                    tl_serializer_block_fill(block, block_index, buffer);
                    block_sequence++;
                    block_index = sizeof(buffer);
                    continue;
                }

                if (block_sequence > 100) {
                    char buffer[3]; itoa(block_sequence, buffer, 10);
                    tl_serializer_block_fill(block, block_index, buffer);
                    block_sequence++;
                    block_index = sizeof(buffer);
                    continue;
                }

                if (block_sequence > 1000) {
                    char buffer[4]; itoa(block_sequence, buffer, 10);
                    tl_serializer_block_fill(block, block_index, buffer);
                    block_sequence++;
                    block_index = sizeof(buffer);
                    continue;
                }

                if (block_sequence > 10000) {
                    char buffer[5]; itoa(block_sequence, buffer, 10);
                    tl_serializer_block_fill(block, block_index, buffer);
                    block_sequence++;
                    block_index = sizeof(buffer);
                }
            } continue;

            case YAML_KEY_TOKEN: {
                yaml_token_delete(&token);
                yaml_parser_scan(&parser, &token);
                if (token.type != YAML_SCALAR_TOKEN) TLFATAL("Expected YAML_SCALAR_TOKEN")

                // -----------------------------------------------------------
                // Copy the scalar content to the 'block'
                // -----------------------------------------------------------
                tl_memory_copy(block, token.data.scalar.value, token.data.scalar.length);
                if (block_index > token.data.scalar.length) {
                    tl_memory_set(block + token.data.scalar.length, 0, block_index - token.data.scalar.length);
                }

                block_index = token.data.scalar.length;
            } break;

            case YAML_SCALAR_TOKEN: {
                if (tl_serializer_read_engine     (property, block, &token)) continue;
                if (tl_serializer_read_application(property, block, &token)) continue;

                TLWARN("Unknown property: %s%s", property, block);
            } break;

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

    TLSTACKPOP
}

static b8 tl_serializer_read_engine(char* property, char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", property, block, token)
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
            runtime->engine.graphics.vsync = tl_char_equals((char*) token->data.scalar.value, "true");
            TLTRACE("runtime->engine.graphics.vsync = %d", runtime->engine.graphics.vsync)
            TLSTACKPOPV(TRUE);
        }
        if (IS_BLOCK_EQ("wireframe")) {
            runtime->engine.graphics.wireframe = tl_char_equals((char*) token->data.scalar.value, "true");
            TLTRACE("runtime->engine.graphics.wireframe = %d", runtime->engine.graphics.wireframe)
            TLSTACKPOPV(TRUE);
        }

        //TODO parse [engine.renderer]
    }

    if (IS_PROP_EQ("engine.simulation.")) {
        if (IS_BLOCK_EQ("step")) {
            u8 step = strtol((char*) token->data.scalar.value, (void*)(token->data.scalar.value + token->data.scalar.length), 10);
            if (step == 0) {
                TLWARN("Failed to read [%s%s] assuming 24", property, block);
                step = 24;
            }

            runtime->engine.simulation.step = 1.0f / (f64) step;
            TLTRACE("runtime->engine.simulation.step = %f", runtime->engine.simulation.step)
            TLSTACKPOPV(TRUE);
        }
    }

    TLSTACKPOPV(FALSE);
}

static b8 tl_serializer_read_application(char* property, char* block, const yaml_token_t *token) {
    TLSTACKPUSHA("%s, %s, 0x%p", property, block, token)
    if (IS_PROP_EQ("application.window.")) {
        if (IS_BLOCK_EQ("title")) {
            runtime->platform.window.title = tl_string_wrap(runtime->arenas.permanent, (char*) token->data.scalar.value);
            TLTRACE("runtime->platform.window.title = %s", token->data.scalar.value)
            TLSTACKPOPV(TRUE);
        }

        if (IS_BLOCK_EQ("size")) {
            if (IS_TOKEN_EQ( "SD")) { runtime->platform.window.size.x = TL_VIDEO_RESOLUTION_SD; }
            if (IS_TOKEN_EQ( "HD")) { runtime->platform.window.size.x = TL_VIDEO_RESOLUTION_HD; }
            if (IS_TOKEN_EQ("FHD")) { runtime->platform.window.size.x = TL_VIDEO_RESOLUTION_FHD; }
            if (IS_TOKEN_EQ("QHD")) { runtime->platform.window.size.x = TL_VIDEO_RESOLUTION_QHD; }
            if (IS_TOKEN_EQ("UHD")) { runtime->platform.window.size.x = TL_VIDEO_RESOLUTION_UHD; }

            runtime->platform.window.size.y = (runtime->platform.window.size.x * 9) / 16;
            TLTRACE("runtime->platform.window.size = %u x %u", runtime->platform.window.size.x, runtime->platform.window.size.y)
            TLSTACKPOPV(TRUE);
        }
    }

    //TODO parse [application.scenes]

    TLSTACKPOPV(FALSE);
}
