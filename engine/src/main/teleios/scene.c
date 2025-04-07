#include "teleios/core.h"
#include "teleios/runtime.h"
#include "teleios/system.h"
#include "teleios/globals.h"

#include <ctype.h>

#define TL_YAML_MAX_SCENE_KEY (22)
#define TL_YAML_MAX_ACTOR_KEY (TL_YAML_MAX_SCENE_KEY + 7 + 3)
#define TL_GLPARAM(p, f, g)             \
    if (tl_string_equals(value, f)) {   \
        p = g;                          \
        BKSTRACE("%s = GL_%s", #p, f)    \
        BKS_STACK_POP                    \
    }

static void tl_scene_load_clear_color(TLMemoryArena *arena, TLString *key, TLString *value);
static void tl_scene_load_depth(TLMemoryArena *arena, TLString *key, TLString *value);
static void tl_scene_load_blend(TLMemoryArena *arena, TLString *key, TLString *value);
static void tl_scene_load_camera(TLMemoryArena *arena, TLString *key, TLString *value);
static void tl_scene_load_actor(TLMemoryArena *arena, const char* scene, TLString *key, TLString *value);
static void tl_scene_load_actor_component(TLMemoryArena *arena, const char* actor, TLString *key, TLString *value);

b8 tl_scene_load(const char* name) {
    BKS_STACK_PUSHA("%s", name)
    BKSDEBUG("Loading scene [%s]", name);
    TLMemoryArena *scrape = tl_memory_arena_create(BKS_MEBI_BYTES(1));

    if (global->properties == NULL || tl_map_length(global->properties) == 0) {
        BKSERROR("Failed to read runtime properties")
        BKS_STACK_POPV(false)
    }

    if (global->application.scene.arena == NULL) {
        global->application.scene.arena = tl_memory_arena_create(BKS_MEBI_BYTES(10));
    } else {
        tl_memory_arena_reset(global->application.scene.arena);
    }

    global->application.scene.name = tl_string_clone(global->application.scene.arena, name);
    // --------------------------------------------------------
    // Sequentially search for the scene with the desired name
    // --------------------------------------------------------
    b8 found = false;
    char* scene = tl_memory_alloc(scrape, TL_YAML_MAX_SCENE_KEY + 1, TL_MEMORY_STRING);

    for (u8 sequence = 0 ; sequence < U8_MAX ; sequence++) {
        snprintf(scene, TL_YAML_MAX_SCENE_KEY + 1, "application.scene.%d.", sequence);

        TLIterator *it = tl_map_keys(global->properties);
        for (TLString* key = tl_iterator_next(it); key != NULL; key = tl_iterator_next(it)) {
            if (!tl_string_start_with(key, scene)) continue;

            const char *value = tl_string(tl_map_get(global->properties, tl_string(key)));
            if (tl_char_equals(value, name)) {
                BKSTRACE("Found scene [%s] at position [%u]", name, sequence)
                found = true;
                break;
            }
        }

        if (found) break;
        tl_memory_set(scene, 0 , TL_YAML_MAX_SCENE_KEY);
    }

    if (!found) { BKSERROR("Scene [%s] not found", name) BKS_STACK_POPV(false) }
    // --------------------------------------------------------
    // Load the scene with the desired name
    // --------------------------------------------------------
    BKSTRACE("Loading scene with prefix [%s]", global->application.scene.prefix)
    TLIterator *it = tl_map_keys(global->properties);
    for (TLString* key = tl_iterator_next(it); key != NULL; key = tl_iterator_next(it)) {
        if (!tl_string_start_with(key, scene)) continue;
        TLString *value = tl_map_get(global->properties, tl_string(key));

        if (tl_string_contains(key, "clear_color")) tl_scene_load_clear_color(scrape, key, value);
        if (tl_string_contains(key, "depth.")) tl_scene_load_depth(scrape, key, value);
        if (tl_string_contains(key, "blend.")) tl_scene_load_blend(scrape, key, value);
        if (tl_string_contains(key, "camera.")) tl_scene_load_camera(scrape, key, value);
        if (tl_string_contains(key, ".actor.")) tl_scene_load_actor(scrape, scene, key, value);
    }

    BKS_STACK_POPV(true)
}

static void tl_scene_load_clear_color(TLMemoryArena *arena, TLString *key, TLString *value) {
    BKS_STACK_PUSHA("%s, %s", tl_string(key), tl_string(value))
    const char *current_pos = tl_string(value);
    char *endptr;

    global->application.scene.graphics.clear_color.x = strtof(current_pos, &endptr);
    if (current_pos == endptr) {
        global->application.scene.graphics.clear_color.x = 0.75f;
        global->application.scene.graphics.clear_color.y = 0.23f;
        global->application.scene.graphics.clear_color.z = 0.75f;
        global->application.scene.graphics.clear_color.w = 1.0f;
        BKSWARN("Failed to read [scene.clear_color.red] assuming magenta");
        BKS_STACK_POP
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
        BKSWARN("Failed to read [scene.clear_color.green] assuming magenta");
        BKS_STACK_POP
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
        BKSWARN("Failed to read [scene.clear_color.blue] assuming magenta");
        BKS_STACK_POP
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
        BKSWARN("Failed to read [scene.clear_color.alpha] assuming magenta");
        BKS_STACK_POP
    }

    BKS_STACK_POP
}

static void tl_scene_load_depth(TLMemoryArena *arena, TLString *key, TLString *value) {
    BKS_STACK_PUSHA("%s, %s", tl_string(key), tl_string(value))
    if (tl_string_contains(key, "depth.enabled")) {
        global->application.scene.graphics.depth_enabled = true;
        BKSTRACE("global->application.scene.graphics.depth_enabled = %d", global->application.scene.graphics.depth_enabled )
        BKS_STACK_POP;
    }

    if (tl_string_contains(key, "depth.function")) {
        TL_GLPARAM(global->application.scene.graphics.depth_function, "LESS", GL_LESS)
        TL_GLPARAM(global->application.scene.graphics.depth_function, "NEVER", GL_NEVER)
        TL_GLPARAM(global->application.scene.graphics.depth_function, "ALWAYS", GL_ALWAYS)
        TL_GLPARAM(global->application.scene.graphics.depth_function, "LEQUAL", GL_LEQUAL)
        TL_GLPARAM(global->application.scene.graphics.depth_function, "GEQUAL", GL_GEQUAL)
        TL_GLPARAM(global->application.scene.graphics.depth_function, "ALWAYS", GL_ALWAYS)
        TL_GLPARAM(global->application.scene.graphics.depth_function, "GREATER", GL_GREATER)
        TL_GLPARAM(global->application.scene.graphics.depth_function, "NOTEQUAL", GL_NOTEQUAL)
        BKS_STACK_POP;
    }

    BKS_STACK_POP
}

static void tl_scene_load_blend(TLMemoryArena *arena, TLString *key, TLString *value) {
    BKS_STACK_PUSHA("%s, %s", tl_string(key), tl_string(value))
    if (tl_string_contains(key, "blend.enabled")) {
        global->application.scene.graphics.blend_enabled = true;
        BKSTRACE("global->application.scene.graphics.blend_enabled = %d", global->application.scene.graphics.blend_enabled )
        BKS_STACK_POP
    }

    if (tl_string_contains(key, "blend.equation") ) {
        TL_GLPARAM(global->application.scene.graphics.blend_equation, "MIN", GL_MIN)
        TL_GLPARAM(global->application.scene.graphics.blend_equation, "MAX", GL_MAX)
        TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_ADD", GL_FUNC_ADD)
        TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_SUBTRACT", GL_FUNC_SUBTRACT)
        TL_GLPARAM(global->application.scene.graphics.blend_equation, "FUNC_REVERSE_SUBTRACT", GL_FUNC_REVERSE_SUBTRACT)
        BKS_STACK_POP
    }

    if (tl_string_contains(key, "blend.function.source")) {
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
        BKS_STACK_POP
    }

    if (tl_string_contains(key, "blend.function.target")) {
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
        BKS_STACK_POP
    }

    BKS_STACK_POP
}

static void tl_scene_load_camera(TLMemoryArena *arena, TLString *key, TLString *value) {
    BKS_STACK_PUSHA("%s, %s", tl_string(key), tl_string(value))
    BKSINFO("Camera : %s", tl_string(key))
    BKS_STACK_POP
}

static TLUlid * tl_scene_create(TLMemoryArena *arena, const char* actor) {
    BKS_STACK_PUSHA("%s", actor)
    u32 empty_index = U32_MAX;
    for (u32 i = 0 ; i < TL_SCENE_MAX_ACTORS ; ++i) {
        if (global->application.ecs.components.yaml[i].prefix == NULL) {
            if (empty_index == U32_MAX) { empty_index = i; }
            continue;
        }

        if (tl_char_equals(tl_string(global->application.ecs.components.yaml[i].prefix), actor)) {
            BKS_STACK_POPV(global->application.ecs.components.yaml[i].entity);
        }
    }

    TLUlid *entity = tl_ecs_create();
    global->application.ecs.components.yaml[empty_index].entity = entity;
    BKSTRACE("global->application.ecs.components.yaml[%d].entity = %s", empty_index, entity->text)
    global->application.ecs.components.yaml[empty_index].prefix = tl_string_clone(global->application.scene.arena, actor);
    BKSTRACE("global->application.ecs.components.yaml[%d].prefix = %s", empty_index, actor)

    BKS_STACK_POPV(entity)
}

static void tl_scene_load_actor(TLMemoryArena *arena, const char* scene, TLString *key, TLString *value) {
    BKS_STACK_PUSHA("%s, %s", tl_string(key), tl_string(value))
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if actor number is within TL_SCENE_MAX_ACTORS
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    char *actor = tl_memory_alloc(arena, TL_YAML_MAX_ACTOR_KEY + 1, TL_MEMORY_STRING);
    char *number = tl_memory_alloc(arena, 3, TL_MEMORY_STRING);
    for (u16 i = 0 ; i < TL_SCENE_MAX_ACTORS ; ++i) {
        tl_char_join(actor, TL_YAML_MAX_ACTOR_KEY, scene, "actor.");
        tl_char_from_i32(number, i, 10);
        tl_char_join(actor, TL_YAML_MAX_ACTOR_KEY, actor, number);

        if (tl_string_start_with(key, actor)) break;
        tl_memory_set(actor, 0, TL_YAML_MAX_ACTOR_KEY);
    }

    // Ignore if it goes beyond TL_SCENE_MAX_ACTORS
    if (*actor == '\0') {
        BKSWARN("Ignoring actor %s", actor)
        BKS_STACK_POP
    }

    TLUlid *entity = tl_scene_create(arena, actor);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // application.ecs.components.name
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    char *name = tl_memory_alloc(arena, TL_YAML_MAX_ACTOR_KEY + 5, TL_MEMORY_STRING);
    tl_char_join(name, TL_YAML_MAX_ACTOR_KEY + 5, actor, ".name");

    if (tl_string_equals(key, name)) {
        u32 empty_index = U32_MAX;
        for (u32 i = 0 ; i < TL_SCENE_MAX_ACTORS ; ++i) {
            if (global->application.ecs.components.name[i].entity == NULL) {
                if (empty_index == U32_MAX) { empty_index = i; }
                continue;
            }

            if (tl_char_equals(global->application.ecs.components.name[i].entity->text, entity->text)) {
                BKSWARN("Element repeated for [%s]", actor)
                break;
            }
        }

        if (empty_index == U32_MAX) {
            BKSWARN("Maximum components reached")
            BKS_STACK_POP
        }

        global->application.ecs.components.name[empty_index].entity = entity;
        BKSTRACE("global->application.ecs.components.name[%d].entity = %s", empty_index, entity->text)
        global->application.ecs.components.name[empty_index].name = tl_string_duplicate(value);
        BKSTRACE("global->application.ecs.components.name[%d].name = %s", empty_index, tl_string(value));
    }

    BKS_STACK_POP
}

static void tl_scene_load_actor_component(TLMemoryArena *arena, const char* actor, TLString *key, TLString *value) {
    BKS_STACK_PUSHA("%s, %s, %s", actor, tl_string(key), tl_string(value))
    BKS_STACK_POP
}
