#include "teleios/core.h"
#include "teleios/core/serializer.h"
#include <string.h>
#include <stdio.h>
#include <yaml.h>

#define TLBLOCK(s) strcmp(block, s) == 0
#define TLPROPERTY(s) strcmp(property, s) == 0
#define TLTOKEN(s) strcmp(token.data.scalar.value, s) == 0

TLApplication* tl_serializer_read(const char *file_name) {
    TLTRACE(">> tl_serializer_read(%s)", file_name)
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        TLERROR("Failed to open %s", file_name);
        TLTRACE("<< tl_serializer_read(%s)", file_name)
        return NULL;
    }

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        fclose(file);
        TLERROR("Failed to initialize parser!");
        TLTRACE("<< tl_serializer_read(%s)", file_name)
        return NULL;
    }

    yaml_parser_set_input_file(&parser, file);
    TLApplication* yaml = tl_memory_alloc(runtime->arenas.permanent, sizeof(TLApplication), TL_MEMORY_SERIALIZER);

    u8 block_index;
    void* block = TLALLOCA(U8_MAX);

    u16 property_index;
    void* property = TLALLOCA(U16_MAX);
    
    yaml_token_t token;
    yaml_token_type_t last_type;
    
    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type) {
            default: last_type = token.type; break;
            // case YAML_NO_TOKEN: { TLINFO("YAML_NO_TOKEN") } break;
            // case YAML_STREAM_START_TOKEN: { TLINFO("YAML_STREAM_START_TOKEN") } break;
            // case YAML_STREAM_END_TOKEN: { TLINFO("YAML_STREAM_END_TOKEN") } break;

            // case YAML_DOCUMENT_END_TOKEN: { TLINFO("YAML_DOCUMENT_END_TOKEN") } break;
            // case YAML_DOCUMENT_START_TOKEN: { TLINFO("YAML_DOCUMENT_START_TOKEN") } break;
            // case YAML_TAG_DIRECTIVE_TOKEN: { TLINFO("YAML_TAG_DIRECTIVE_TOKEN") } break;
            // case YAML_VERSION_DIRECTIVE_TOKEN: { TLINFO("YAML_VERSION_DIRECTIVE_TOKEN") } break;
            
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
                    if (*((char*)(property + i))  == '.') {
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

            // case YAML_BLOCK_ENTRY_TOKEN: { TLINFO("YAML_BLOCK_ENTRY_TOKEN") } break;
            // case YAML_FLOW_ENTRY_TOKEN: { TLINFO("YAML_FLOW_ENTRY_TOKEN") } break;
            // case YAML_KEY_TOKEN: { TLINFO("YAML_KEY_TOKEN") } break;
            // case YAML_VALUE_TOKEN: { TLINFO("YAML_VALUE_TOKEN")} break;

            // case YAML_ALIAS_TOKEN: { TLINFO("YAML_ALIAS_TOKEN") } break;
            // case YAML_ANCHOR_TOKEN: { TLINFO("YAML_ANCHOR_TOKEN") } break;
            // case YAML_TAG_TOKEN: { TLINFO("YAML_TAG_TOKEN") } break;
            case YAML_SCALAR_TOKEN: { 
                if (last_type == YAML_KEY_TOKEN) {
                    block_index = 0;
                    tl_memory_set(block, 0, U8_MAX); 
                    
                    block_index = token.data.scalar.length;
                    tl_memory_copy(block, token.data.scalar.value, token.data.scalar.length);
                    continue;
                }

                if (TLPROPERTY("application.window.")) { 
                    if (TLBLOCK("title")) {
                        yaml->window.title = token.data.scalar.value; 
                        continue;
                    }

                    if (TLBLOCK("size")) {
                        if (TLTOKEN("SD")) { yaml->window.resolution = TL_VIDEO_RESOLUTION_SD; }
                        if (TLTOKEN("HD")) { yaml->window.resolution = TL_VIDEO_RESOLUTION_HD; }
                        if (TLTOKEN("FHD")) { yaml->window.resolution = TL_VIDEO_RESOLUTION_FHD; }
                        if (TLTOKEN("QHD")) { yaml->window.resolution = TL_VIDEO_RESOLUTION_QHD; }
                        if (TLTOKEN("UHD")) { yaml->window.resolution = TL_VIDEO_RESOLUTION_UHD; }
                        continue;
                    }
                }

                if (TLPROPERTY("application.simulation.")) {
                    if (TLBLOCK("step")) {
                        yaml->simulation.step = strtol(token.data.scalar.value, (void*)(token.data.scalar.value + token.data.scalar.length), 10);
                        if (yaml->simulation.step == 0) {
                            TLWARN("Failed to read [%s%s] assuming 24", property, block);
                            yaml->simulation.step = 24;
                        }
                        continue;
                    }
                }

            } break;
        }
        
        if (token.type != YAML_STREAM_END_TOKEN) { 
            yaml_token_delete(&token); 
        }
    } while (token.type != YAML_STREAM_END_TOKEN);

    yaml_token_delete(&token);
    yaml_parser_delete(&parser);

    TLTRACE("<< tl_serializer_read(%s)", file_name)
    return yaml;
}
