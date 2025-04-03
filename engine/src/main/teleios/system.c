#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/system.h"
#include "teleios/globals.h"
#include <ctype.h>

// #####################################################################################################################
//
//                                                       ECS
//
// #####################################################################################################################
static TLUlidGenerator *generator;

TLUlid* tl_ecs_create(void) {
    TL_STACK_PUSH
    if (generator == NULL) {
        generator = tl_ulid_generator_init(global->application.scene.arena, ULID_PARANOID);
    }

    for (u16 i = 0; i < TL_SCENE_MAX_ACTORS; ++i) {
        if (global->application.ecs.entity[i] != NULL) continue;
        global->application.ecs.entity[i] = tl_ulid_generate(global->application.scene.arena, generator);
        TL_STACK_POPV(global->application.ecs.entity[i])
    }

    TLWARN("Failed to generate entity: array is full")
    TL_STACK_POPV(NULL)
}

void tl_ecs_destroy(TLUlid *entity) {
    TL_STACK_PUSHA("0x%p", entity)
    if (entity == NULL) TL_STACK_POP

    for (u16 i = 0; i < TL_SCENE_MAX_ACTORS; ++i) {
        if (global->application.ecs.entity[i] != NULL && tl_char_equals(global->application.ecs.entity[i]->text, entity->text)) {
            global->application.ecs.entity[i] = NULL;
            TL_STACK_POP
        }
    }

    TLWARN("Failed to destroy entity: not found")
    TL_STACK_POP
}
/*
#define TL_ECS_ATTACH_FUNCTION(type)                                                \
    static u16 tl_ecs_attach_##type(TLUlid *entity) {                               \
        TL_STACK_PUSHA("0x%p, %d", entity)                                          \
        for (u16 i = 0; i < TL_SCENE_MAX_ACTORS; ++i) {                             \
            if (global->application.ecs.components.type[i].entity == NULL) {        \
                global->application.ecs.components.type[i].entity = entity;         \
                TL_STACK_POPV(i)                                                    \
            }                                                                       \
        }                                                                           \
                                                                                    \
        TLWARN("Failed to generate ecs.components.%s: array is full", #type)        \
        TL_STACK_POPV(U16_MAX) \
    }

TL_ECS_ATTACH_FUNCTION(yaml)
TL_ECS_ATTACH_FUNCTION(name)

u16 tl_ecs_attach(TLUlid *entity, const TLEcsComponentType type) {
    TL_STACK_PUSHA("0x%p, %d", entity, type)
    u16 index = U16_MAX;
    switch (type) {
        case TL_ECS_COMPONENT_YAML: index = tl_ecs_attach_yaml(entity); break;
        case TL_ECS_COMPONENT_NAME: index = tl_ecs_attach_name(entity); break;
    }

    TL_STACK_POPV(index);
}
*/
// #####################################################################################################################
//
//                                                     SCENE
//
// #####################################################################################################################
#define TL_GLPARAM(p, f, g)             \
        if (tl_char_equals(value, f)) { \
        p = g;                          \
        TLTRACE("%s = GL_%s", #p, f)    \
        TL_STACK_POP                    \
}

static void tl_serializer_load_scene(const char *prefix, const char *element, const char *value) {
    TL_STACK_PUSHA("%s, %s, %s", prefix, element, value)

    // Ensure the right [application.scenes.#] is being parsed
    if (!tl_char_start_with(prefix, global->application.scene.prefix)) TL_STACK_POP

    // String used to check if property is the desired key
    char buffer[TL_YAML_PROPERTY_MAX_SIZE];
    const u8 prefix_length = tl_char_length(prefix);

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
            TL_STACK_POP
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
            TL_STACK_POP
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
            TL_STACK_POP
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
            TL_STACK_POP
        }

        TL_STACK_POP
    }
    // ----------------------------------------------------------------
    //  application.scenes.#.depth
    // ----------------------------------------------------------------
    tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
    tl_char_join(buffer, TL_YAML_PROPERTY_MAX_SIZE, global->application.scene.prefix, "depth.");
    if ( tl_char_equals(buffer, prefix)) {
        if (tl_char_equals(element, "enabled") ) {
            global->application.scene.graphics.depth_enabled = true;
            TLTRACE("global->application.scene.graphics.depth_enabled = %d", global->application.scene.graphics.depth_enabled )
            TL_STACK_POP
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

        TLWARN("depth")
        TL_STACK_POP
    }
    // ----------------------------------------------------------------
    //  application.scenes.#.blend
    // ----------------------------------------------------------------
    tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
    tl_char_join(buffer, TL_YAML_PROPERTY_MAX_SIZE, global->application.scene.prefix, "blend.");
    if (tl_char_start_with(prefix, buffer)) {
        if (tl_char_equals(element, "enabled") ) {
            global->application.scene.graphics.blend_enabled = true;
            TLTRACE("global->application.scene.graphics.blend_enabled = %d", global->application.scene.graphics.blend_enabled )
            TL_STACK_POP
        }

        if (tl_char_equals(element, "equation") ) {
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "MIN", GL_MIN)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "MAX", GL_MAX)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_ADD", GL_FUNC_ADD)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_SUBTRACT", GL_FUNC_SUBTRACT)
            TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_REVERSE_SUBTRACT", GL_FUNC_REVERSE_SUBTRACT)
            TL_STACK_POP
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
                TL_STACK_POP
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
                TL_STACK_POP
            }
        }

        TLWARN("blend")
        TL_STACK_POP
    }
    // ----------------------------------------------------------------
    //  application.scenes.#.camera
    // ----------------------------------------------------------------
    tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
    tl_char_join(buffer, TL_YAML_PROPERTY_MAX_SIZE, global->application.scene.prefix, "camera.");
    if (tl_char_start_with(prefix, buffer)) {
        TLINFO("Camera : %s", prefix)
        TL_STACK_POP
    }

    // ----------------------------------------------------------------
    //  application.scenes.#.actors.####.
    // ----------------------------------------------------------------
    if (prefix_length <= 33) {
        tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
        tl_char_join(buffer, TL_YAML_PROPERTY_MAX_SIZE, global->application.scene.prefix, "actors.");
        if (!tl_char_start_with(prefix, buffer)) TL_STACK_POP

        TLUlid *entity = NULL;

        {
            u32 empty_index = U32_MAX;
            for (u32 i = 0 ; i < TL_SCENE_MAX_ACTORS ; ++i) {
                if (global->application.ecs.components.yaml[i].prefix == NULL) {
                    if (empty_index == U32_MAX) { empty_index = i; }
                    continue;
                }

                if (tl_char_equals(tl_string(global->application.ecs.components.yaml[i].prefix), prefix)) {
                    entity = global->application.ecs.components.yaml[i].entity;
                    break;
                }
            }

            if (empty_index > TL_SCENE_MAX_ACTORS) {
                TLWARN("Maximum components reached")
                TL_STACK_POP
            }

            if (entity == NULL) {
                entity = tl_ecs_create();
                global->application.ecs.components.yaml[empty_index].entity = entity;
                TLTRACE("global->application.ecs.components.yaml[%d].entity = %s", empty_index, entity->text)
                global->application.ecs.components.yaml[empty_index].prefix = tl_string_clone(global->platform.arena, prefix);
                TLTRACE("global->application.ecs.components.yaml[%d].prefix = %s", empty_index, prefix)

            }
        }

        tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
        if (tl_char_equals(element, "name") ) {
            u32 empty_index = U32_MAX;
            for (u32 i = 0 ; i < TL_SCENE_MAX_ACTORS ; ++i) {
                if (global->application.ecs.components.name[i].entity == NULL) {
                    if (empty_index == U32_MAX) { empty_index = i; }
                    continue;
                }

                if (tl_char_equals(global->application.ecs.components.name[i].entity->text, entity->text)) {
                    TLWARN("Element [%s] repeated for prefix [%s]", element, prefix)
                    break;
                }
            }

            if (empty_index > TL_SCENE_MAX_ACTORS) {
                TLWARN("Maximum components reached")
                TL_STACK_POP
            }

            global->application.ecs.components.name[empty_index].entity = entity;
            TLTRACE("global->application.ecs.components.name[%d].entity = %s", empty_index, entity->text)
            global->application.ecs.components.name[empty_index].name = tl_string_clone(global->platform.arena, prefix);
            TLTRACE("global->application.ecs.components.name[%d].name = %s", empty_index, value)
        }
    }

    // ----------------------------------------------------------------
    //  application.scenes.#.actors.####.components.###.
    // ----------------------------------------------------------------
    tl_memory_set(buffer, 0, TL_YAML_PROPERTY_MAX_SIZE);
    if (tl_char_contains(prefix, ".components.")) {
        // =========================================
        // Find the entity id
        // =========================================
        TLUlid *entity = NULL;
        for (u32 i = 0 ; i < TL_SCENE_MAX_ACTORS; ++i) {
            if (global->application.ecs.components.yaml[i].prefix == NULL) continue;
            if (tl_string_start_with(global->application.ecs.components.yaml[i].prefix, prefix)) {
                entity = global->application.ecs.components.yaml[i].entity;
                break;
            }
        }

        if (entity == NULL) {
            TLWARN("Entity not found for prefix [%s]", prefix)
            TL_STACK_POP
        }

        if (tl_char_equals(element, "name")) {
            if (tl_char_equals(value, "COMPONENT_SCRIPT")) {
                for (u32 i = 0 ; i < TL_SCENE_MAX_ACTORS * TL_SCENE_MAX_ACTOR_SCRIPTS; ++i) {
                    if (global->application.ecs.components.script[i].entity != NULL) continue;
                    global->application.ecs.components.script[i].entity = entity;
                    global->application.ecs.components.script[i].prefix = tl_string_clone(global->platform.arena, prefix);
                }
            }
        }


        TLINFO("Component : %s", prefix)
    }

    TL_STACK_POP
}

static void tl_serializer_find_scene(const char *prefix, const char  *element, const char *value) {
    TL_STACK_PUSHA("%s, %s, %s", prefix, element, value)

    if (!tl_char_equals(prefix, global->application.scene.prefix)) TL_STACK_POP
    if (!tl_char_equals(element, "name")) TL_STACK_POP
    if (!tl_char_equals(value, tl_string(global->application.scene.name))) TL_STACK_POP

    global->application.scene.found = true;
    TL_STACK_POP
}

b8 tl_scene_load(const char* name) {
    TL_STACK_PUSHA("%s", name)
    TLDEBUG("Loading scene [%s]", name);

    if (global->application.scene.arena == NULL) {
        global->application.scene.arena = tl_memory_arena_create(TL_MEBI_BYTES(10));
    } else {
        tl_memory_arena_reset(global->application.scene.arena);
    }

    global->application.scene.name = tl_string_clone(global->application.scene.arena, name);
    // --------------------------------------------------------
    // Sequentially search for the scene with the desired name
    // --------------------------------------------------------
    global->application.scene.found = false;
    for (u8 sequence = 0 ; sequence < U8_MAX ; sequence++) {
        tl_memory_set(global->application.scene.prefix, 0, U8_MAX);

        snprintf(global->application.scene.prefix, U8_MAX, "application.scenes.%d.", sequence);
        tl_serializer_walk(tl_serializer_find_scene);
        if (global->application.scene.found) {
            TLTRACE("Found scene [%s] at position [%u]", name, sequence)
            break;
        }
    }

    if (!global->application.scene.found) {
        TLERROR("Scene [%s] not found", name)
        TL_STACK_POPV(false)
    }
    // --------------------------------------------------------
    // Load the scene with the desired name
    // --------------------------------------------------------
    TLTRACE("Loading scene with prefix [%s]", global->application.scene.prefix)
    tl_serializer_walk(tl_serializer_load_scene);
    TL_STACK_POPV(true)
}
