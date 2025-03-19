#include "teleios/core.h"
#include "teleios/core/serializer.h"
#include <string.h>
#include <stdio.h>
#include <yaml.h>

#define TLBLOCK(s) strcmp(block, s) == 0
#define TLPROPERTY(s) strcmp(property, s) == 0
#define TLTOKEN(s) strcmp(token.data.scalar.value, s) == 0

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

    u8 block_index;
    char block[U8_MAX];

    u16 property_index;
    char property[U16_MAX];
    
    yaml_token_t token;
    yaml_token_type_t last_type;
    
    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type) {
            default: last_type = token.type; break;
            case YAML_FLOW_MAPPING_START_TOKEN: 
            case YAML_FLOW_SEQUENCE_START_TOKEN: 
            case YAML_BLOCK_MAPPING_START_TOKEN: 
            case YAML_BLOCK_SEQUENCE_START_TOKEN: { 
                if (block_index == 0) continue;

                tl_memory_copy(property + property_index, block, block_index); 
                property_index += block_index;
                
                tl_memory_copy(property + property_index, ".", 1);
                property_index++;
            } break;
            
            case YAML_BLOCK_END_TOKEN:
            case YAML_FLOW_SEQUENCE_END_TOKEN:
            case YAML_FLOW_MAPPING_END_TOKEN: { 
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
             } break;

            case YAML_SCALAR_TOKEN: { 
                if (last_type == YAML_KEY_TOKEN) {
                    // Override the value, erasing the remaining used characters if needed
                    tl_memory_copy(block, token.data.scalar.value, token.data.scalar.length);
                    if (block_index > token.data.scalar.length) {
                        tl_memory_set(block + token.data.scalar.length, 0, block_index - token.data.scalar.length);
                    }

                    block_index = token.data.scalar.length;
                    continue;
                }
                // -------------------------------------------------------------------------
                //
                // Parse the [engine] properties
                //
                // -------------------------------------------------------------------------
                if (TLPROPERTY("engine.graphics.")) {
                    if (TLBLOCK("vsync")) {
                        runtime->graphics.vsync = tl_char_equals((char*) token.data.scalar.value, "true");
                    }
                    if (TLBLOCK("wireframe")) {
                        runtime->graphics.wireframe = tl_char_equals((char*) token.data.scalar.value, "true");
                    }

                    //TODO parse [engine.renderer]
                }
                // -------------------------------------------------------------------------
                //
                // Parse the [application] properties
                //
                // -------------------------------------------------------------------------
                if (TLPROPERTY("application.window.")) { 
                    if (TLBLOCK("title")) {
                        runtime->platform.window.title = tl_string_wrap(runtime->arenas.permanent, (char*) token.data.scalar.value);
                        continue;
                    }

                    if (TLBLOCK("size")) {
                        if (TLTOKEN( "SD")) { runtime->platform.window.width = TL_VIDEO_RESOLUTION_SD; }
                        if (TLTOKEN( "HD")) { runtime->platform.window.width = TL_VIDEO_RESOLUTION_HD; }
                        if (TLTOKEN("FHD")) { runtime->platform.window.width = TL_VIDEO_RESOLUTION_FHD; }
                        if (TLTOKEN("QHD")) { runtime->platform.window.width = TL_VIDEO_RESOLUTION_QHD; }
                        if (TLTOKEN("UHD")) { runtime->platform.window.width = TL_VIDEO_RESOLUTION_UHD; }

                        runtime->platform.window.height = (runtime->platform.window.width * 9) / 16;
                        continue;
                    }
                }

                if (TLPROPERTY("application.simulation.")) {
                    if (TLBLOCK("step")) {
                        u8 step = strtol((char*) token.data.scalar.value, (void*)(token.data.scalar.value + token.data.scalar.length), 10);
                        if (step == 0) {
                            TLWARN("Failed to read [%s%s] assuming 24", property, block);
                            runtime->simulation.step = 1.0f / 24.0;
                        } else {
                            runtime->simulation.step = 1.0f / (f64) step;
                        }

                        continue;
                    }
                }


                //TODO parse [application.scenes]

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
