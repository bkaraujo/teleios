#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/globals.h"

#include <ctype.h>

#define TL_GLPARAM(p, f, g)             \
        if (tl_char_equals(value, f)) { \
        p = g;                          \
        TLTRACE("%s = GL_%s", #p, f)    \
        TLSTACKPOP                      \
}

static void tl_serializer_load_scene(const char *prefix, const char *element, const char *value, u64 length) {
    TLSTACKPUSHA("%s, %s, %s", prefix, element, value)

    // Ensure the right [application.scenes.#] is being parsed
    if ( ! tl_char_start_with(prefix, global->application.scene.prefix) ) TLSTACKPOP

    // String used to check if property is the desired key
    char buffer[TL_YAML_PROPERTY_MAX_SIZE];

    // ----------------------------------------------------------------
    //  application.scenes.#.clear_color
    // ----------------------------------------------------------------
    if (tl_char_equals(element, "clear_color")) {
        const char *current_pos = value;
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
    tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
    tl_char_join(buffer, TL_YAML_PROPERTY_MAX_SIZE, global->application.scene.prefix, "depth.");
    if ( tl_char_equals(buffer, prefix)) {
        if (tl_char_equals(element, "enabled") ) {
            global->application.scene.graphics.depth_enabled = TRUE;
            TLTRACE("global->application.scene.graphics.depth_enabled = %d", global->application.scene.graphics.depth_enabled )
            TLSTACKPOP
        }

        if (tl_char_equals(element, "function") ) {
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
    tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
    tl_char_join(buffer, TL_YAML_PROPERTY_MAX_SIZE, global->application.scene.prefix, "blend.");
    if (tl_char_start_with(prefix, buffer)) {
        if (tl_char_equals(element, "enabled") ) {
            global->application.scene.graphics.blend_enabled = TRUE;
            TLTRACE("global->application.scene.graphics.blend_enabled = %d", global->application.scene.graphics.blend_enabled )
            TLSTACKPOP
        }

        if (tl_char_equals(element, "equation") ) {
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "MIN", GL_MIN)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "MAX", GL_MAX)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_ADD", GL_FUNC_ADD)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_SUBTRACT", GL_FUNC_SUBTRACT)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_REVERSE_SUBTRACT", GL_FUNC_REVERSE_SUBTRACT)
        }

        tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
        tl_char_join(buffer, TL_YAML_PROPERTY_MAX_SIZE, global->application.scene.prefix, "blend.function.");
        if (tl_char_equals(prefix, buffer)) {
            if (tl_char_equals(element, "source") ) {
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

            if (tl_char_equals(element, "target") ) {
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

static void tl_serializer_find_scene(const char *prefix, const char *element, const char *value, u64 length) {
    TLSTACKPUSHA("%s, %s, %s", prefix, element, value)

    if ( ! tl_char_equals(prefix, global->application.scene.prefix) ) TLSTACKPOP
    if ( ! tl_char_equals(element, "name") ) TLSTACKPOP
    if ( ! tl_char_equals(value, tl_string(global->application.scene.name))) TLSTACKPOP

    global->application.scene.found = TRUE;
    TLSTACKPOP
}

b8 tl_scene_load(const char* name) {
    TLSTACKPUSHA("%s", name)
    TLDEBUG("Loading scene [%s]", name);
    tl_memory_arena_reset(global->application.scene.arena);
    global->application.scene.name = tl_string_clone(global->application.scene.arena, name);
    // --------------------------------------------------------
    // Sequentially search for the scene with the desired name
    // --------------------------------------------------------
    global->application.scene.found = FALSE;
    for (u8 sequence = 0 ; sequence < U8_MAX ; sequence++) {
        tl_memory_set(global->application.scene.prefix, 0, U8_MAX);

        snprintf(global->application.scene.prefix, U8_MAX, "application.scenes.%d.", sequence);
        tl_serializer_walk(tl_serializer_find_scene);
        if (global->application.scene.found) {
            TLDEBUG("Found scene [%s] at position [%u]", name, sequence)
            break;
        }
    }

    if (!global->application.scene.found) {
        TLERROR("Scene [%s] not found", name)
        TLSTACKPOPV(FALSE)
    }
    // --------------------------------------------------------
    // Load the scene with the desired name
    // --------------------------------------------------------
    tl_serializer_walk(tl_serializer_load_scene);
    TLSTACKPOPV(TRUE)
}
