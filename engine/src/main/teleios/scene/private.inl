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

static void tl_scene_init_lua(TLScene* scene) {
    TL_PROFILER_PUSH_WITH("scene=%s", tl_string_cstr(scene->name));

    lua_State* state = luaL_newstate();
    if (state == NULL) TLFATAL("Failed to create Lua state for scene '%s'", tl_string_cstr(scene->name));

    luaL_openlibs(state);               // Registra funções padrão LUA
    tl_script_input_register(state);    // Registra API interna

    scene->lua_state = (void*)state;

    TL_PROFILER_POP
}

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
// Funções Internas - Configuração de Graphics
// ============================================================================

static TLString* tl_scene_build_config_path(TLAllocator* allocator, const u32 index, const char* suffix) {
    TLStringBuilder* builder = tl_string_builder_create(allocator, 128);
    tl_string_builder_append_cstr(builder, "application.scene.");

    const TLString* idx_str = tl_number_u32_to_char(allocator, index, 10);
    tl_string_builder_append(builder, idx_str);

    tl_string_builder_append_cstr(builder, ".");
    tl_string_builder_append_cstr(builder, suffix);

    return tl_string_builder_build(builder);
}

static void tl_scene_apply_graphics_config(TLScene* scene) {
    TL_PROFILER_PUSH_WITH("scene=0x%p", scene);

    TLAllocator* allocator = tl_memory_allocator_create(TL_KIBI_BYTES(2), TL_ALLOCATOR_LINEAR);

    // Clear Color
    TLString* path = tl_scene_build_config_path(allocator, scene->index, "graphics.clear_color");
    TLString* clear_color_str = tl_config_get(tl_string_cstr(path));
    tl_string_destroy(path);

    if (clear_color_str != NULL) {
        f32 r, g, b, a;
        if (sscanf(tl_string_cstr(clear_color_str), "%f, %f, %f, %f", &r, &g, &b, &a) == 4) {
            TLDEBUG("Setting clear color: (%.2f, %.2f, %.2f, %.2f)", r, g, b, a);
            tl_graphics_set_clear_color(r, g, b, a);
        }
        tl_string_destroy(clear_color_str);
    }

    // Depth Test
    path = tl_scene_build_config_path(allocator, scene->index, "graphics.depth.enabled");
    TLString* depth_enabled_str = tl_config_get(tl_string_cstr(path));
    tl_string_destroy(path);

    if (depth_enabled_str != NULL) {
        TLString* upper = tl_string_to_upper(depth_enabled_str);
        b8 enabled = tl_string_equals_cstr(upper, "TRUE");
        tl_string_destroy(upper);
        tl_string_destroy(depth_enabled_str);

        TLDEBUG("Depth test: %s", enabled ? "enabled" : "disabled");

        if (enabled) {
            tl_graphics_enable_depth();

            // Depth function
            path = tl_scene_build_config_path(allocator, scene->index, "graphics.depth.function");
            TLString* depth_func_str = tl_config_get(tl_string_cstr(path));
            tl_string_destroy(path);

            if (depth_func_str != NULL) {
                TLDEBUG("Depth function: %s", tl_string_cstr(depth_func_str));
                tl_graphics_set_depth_function(depth_func_str);
                tl_string_destroy(depth_func_str);
            }
        } else {
            tl_graphics_disable_depth();
        }
    }

    // Blend
    path = tl_scene_build_config_path(allocator, scene->index, "graphics.blend.enabled");
    TLString* blend_enabled_str = tl_config_get(tl_string_cstr(path));
    tl_string_destroy(path);

    b8 blend_enabled = false;
    if (blend_enabled_str != NULL) {
        TLString* upper = tl_string_to_upper(blend_enabled_str);
        blend_enabled = tl_string_equals_cstr(upper, "TRUE");
        tl_string_destroy(upper);
        tl_string_destroy(blend_enabled_str);
    }

    if (blend_enabled) {
        TLDEBUG("Blend: enabled");
        tl_graphics_enable_blend();

        // Blend equation
        path = tl_scene_build_config_path(allocator, scene->index, "graphics.blend.equation");
        TLString* blend_eq_str = tl_config_get(tl_string_cstr(path));
        tl_string_destroy(path);

        if (blend_eq_str != NULL) {
            TLDEBUG("Blend equation: %s", tl_string_cstr(blend_eq_str));
            tl_graphics_set_blend_equation(blend_eq_str);
            tl_string_destroy(blend_eq_str);
        }

        // Blend function (source, target)
        path = tl_scene_build_config_path(allocator, scene->index, "graphics.blend.function.source");
        TLString* blend_src_str = tl_config_get(tl_string_cstr(path));
        tl_string_destroy(path);

        path = tl_scene_build_config_path(allocator, scene->index, "graphics.blend.function.target");
        TLString* blend_dst_str = tl_config_get(tl_string_cstr(path));
        tl_string_destroy(path);

        if (blend_src_str != NULL && blend_dst_str != NULL) {
            TLDEBUG("Blend function: src=%s, dst=%s",
                    tl_string_cstr(blend_src_str), tl_string_cstr(blend_dst_str));
            tl_graphics_set_blend_function(blend_src_str, blend_dst_str);
        }

        if (blend_src_str != NULL) tl_string_destroy(blend_src_str);
        if (blend_dst_str != NULL) tl_string_destroy(blend_dst_str);
    } else {
        TLDEBUG("Blend: disabled");
        tl_graphics_disable_blend();
    }

    tl_memory_allocator_destroy(allocator);
    TL_PROFILER_POP
}

static void tl_scene_apply_camera_config(TLScene* scene) {
    TL_PROFILER_PUSH_WITH("scene=0x%p", scene);

    // TODO: Implementar configuração de câmera quando o sistema de câmera estiver pronto
    TLDEBUG("Camera configuration for scene '%s' (not yet implemented)", tl_string_cstr(scene->name));

    TL_PROFILER_POP
}


#endif
