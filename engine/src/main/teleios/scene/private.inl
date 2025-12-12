#ifndef __TELEIOS_SCENE_PRIVATE__
#define __TELEIOS_SCENE_PRIVATE__

#include "teleios/teleios.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// ============================================================================
// Funções Internas - Inicialização Lua e Execução de Scripts
// ============================================================================

#include "teleios/script/input.inl"

// ============================================================================

// ============================================================================
static void tl_scene_init_lua(TLScene* scene) {
    TL_PROFILER_PUSH_WITH("scene=%s", tl_string_cstr(scene->name));

    lua_State* state = luaL_newstate();
    if (state == NULL) TLFATAL("Failed to create Lua state for scene '%s'", tl_string_cstr(scene->name));

    luaL_openlibs(state);               // Registra funções padrão LUA
    tl_script_input_register(state);    // Registra API interna

    scene->lua_state = (void*)state;

    TL_PROFILER_POP
}
// ============================================================================

// ============================================================================
static b8 tl_scene_execute_script(const TLScene* scene, const TLString* script_path) {
    TL_PROFILER_PUSH_WITH("scene=%s, script=%s", tl_string_cstr(scene->name), tl_string_cstr(script_path));

    if (scene->lua_state == NULL) {
        TLERROR("Cannot execute script: Lua state is NULL for scene '%s'", tl_string_cstr(scene->name));
        TL_PROFILER_POP_WITH(false)
    }

    if (!tl_filesystem_exists(script_path)) {
        TL_PROFILER_POP_WITH(false)
    }

    lua_State* state = (lua_State*)scene->lua_state;
    if (luaL_dofile(state, tl_string_cstr(script_path)) != LUA_OK) {
        const char* error = lua_tostring(state, -1);
        TLERROR("Lua error in '%s': %s", tl_string_cstr(script_path), error);
        lua_pop(state, 1);
        TL_PROFILER_POP_WITH(false)
    }

    TL_PROFILER_POP_WITH(true)
}
// ============================================================================
// Helper para ler script path do YAML
// ============================================================================
static TLString* tl_config_read_script_path(TLAllocator* allocator, const TLString* path_prefix, const char* script_name) {
    TLStringBuilder* builder = tl_string_builder_create(allocator, tl_string_length(path_prefix) + 20);
    tl_string_builder_append(builder, path_prefix);
    tl_string_builder_append_cstr(builder, ".script.");
    tl_string_builder_append_cstr(builder, script_name);

    return tl_config_get(tl_string_cstr(tl_string_builder_build(builder)));
}
// ============================================================================
// Funções Internas - Configuração de Graphics
// ============================================================================
static const char* tl_scene_config(TLAllocator* allocator, const TLScene* scene, const char* suffix) {
    const unsigned size_prefix = tl_string_length(scene->config);
    const unsigned size_suffix = tl_string_length_cstr(suffix);
    TLStringBuilder* builder = tl_string_builder_create(allocator,  size_prefix + size_suffix);

    tl_string_builder_append_cstr(builder, tl_string_cstr(scene->config));
    tl_string_builder_append_cstr(builder, ".");
    tl_string_builder_append_cstr(builder, suffix);

    return tl_string_cstr(tl_string_builder_build(builder));
}
// ============================================================================

// ============================================================================
static void tl_scene_apply_graphics_config(TLAllocator* allocator, TLScene* scene) {
    TL_PROFILER_PUSH_WITH("allocator=0x%p, scene=0x%p", allocator, scene);
    // ============================================================================
    // Clear Color
    // ============================================================================
    const TLString* clear_color_str = tl_config_get(tl_scene_config(allocator, scene, "graphics.clear_color"));
    if (clear_color_str != NULL) {
        const vec4s vector = tl_number_vec4s_from_string(clear_color_str);
        tl_graphics_set_clear_color(vector.r, vector.g, vector.b, vector.a);
    }
    // ============================================================================
    // Depth Test
    // ============================================================================
    if (tl_config_get_b8(tl_scene_config(allocator, scene, "graphics.depth.enabled"))) {
        TLTRACE("Depth: enabled");
        tl_graphics_enable_depth();

        // Depth function
        const TLString* depth_func_str = tl_config_get(tl_scene_config(allocator, scene, "graphics.depth.function"));
        if (depth_func_str != NULL) {
            TLTRACE("Depth function: %s", tl_string_cstr(depth_func_str));
            tl_graphics_set_depth_function(depth_func_str);
        }
    } else {
        TLTRACE("Depth: disabled");
        tl_graphics_disable_depth();
    }
    // ============================================================================
    // Blend
    // ============================================================================
    if (tl_config_get_b8(tl_scene_config(allocator, scene, "graphics.blend.enabled"))) {
        TLTRACE("Blend: enabled");
        tl_graphics_enable_blend();

        // Blend equation
        const TLString* blend_eq_str = tl_config_get(tl_scene_config(allocator, scene, "graphics.blend.equation"));
        if (blend_eq_str != NULL) {
            TLTRACE("Blend equation: %s", tl_string_cstr(blend_eq_str));
            tl_graphics_set_blend_equation(blend_eq_str);
        }

        // Blend function (source, target)
        const TLString* blend_src_str = tl_config_get(tl_scene_config(allocator, scene, "graphics.blend.source"));
        const TLString* blend_dst_str = tl_config_get(tl_scene_config(allocator, scene, "graphics.blend.target"));
        if (blend_src_str != NULL && blend_dst_str != NULL) {
            TLTRACE("Blend function: src=%s, dst=%s", tl_string_cstr(blend_src_str), tl_string_cstr(blend_dst_str));
            tl_graphics_set_blend_function(blend_src_str, blend_dst_str);
        }
    } else {
        TLTRACE("Blend: disabled");
        tl_graphics_disable_blend();
    }

    TL_PROFILER_POP
}

#endif
