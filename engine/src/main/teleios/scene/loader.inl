#ifndef __TELEIOS_SCENE_LOADER__
#define __TELEIOS_SCENE_LOADER__

#include "teleios/teleios.h"
#include "teleios/scene/private.inl"

TLScene* tl_config_get_scene(const TLString* name) {
    TL_PROFILER_PUSH_WITH("name=0x%p", name)

    if (name == NULL) { TLERROR("Attempted to get scene with NULL name"); TL_PROFILER_POP_WITH(NULL) }

    // ==========================================
    // 1. Criação da Cena
    // ==========================================
    TLScene* scene = tl_memory_alloc(global->allocator, TL_MEMORY_SCENE, sizeof(TLScene));
    TLAllocator* allocator = tl_memory_allocator_create(TL_KIBI_BYTES(4), TL_ALLOCATOR_LINEAR);

    // ==========================================
    //
    // ==========================================
    // Resolves to application.scene.#
    unsigned index = 0;
    for (; index <= U8_MAX; ++index) {
        // YAML flat configuration property
        const TLString* property = NULL;

        // ==========================================
        // Construir path para o campo name
        // ==========================================
        {
            TLStringBuilder* builder = tl_string_builder_create(allocator, 128);
            tl_string_builder_append_cstr(builder, "application.scene.");
            const TLString* sIndex = tl_number_u32_to_char(allocator, index, 10);
            tl_string_builder_append(builder, sIndex);

            scene->config = tl_string_builder_build(builder);

            tl_string_builder_append_cstr(builder, ".name");
            property = tl_string_builder_build(builder);
        }

        // ==========================================
        // Verifica se o nome da cena é o desejado
        // ==========================================
        TLString* scene_name = tl_config_get(tl_string_cstr(property));
        if (scene_name == NULL) { break; }
        if (tl_string_equals(scene_name, name)) {
            tl_string_destroy(scene_name);
            break;
        }

        tl_string_destroy(scene_name);
    }

    if (index == U8_MAX) {
        tl_memory_allocator_destroy(allocator);
        tl_memory_free(global->allocator, scene);
        TLWARN("Scene '%s' not found in application.yml", tl_string_cstr(name));
        TL_PROFILER_POP_WITH(NULL)
    }

    TLDEBUG("Found scene '%s' at index %d", tl_string_cstr(name), index);
    scene->allocator = tl_memory_allocator_create(0, TL_ALLOCATOR_DYNAMIC);
    scene->name = tl_string_create(scene->allocator, tl_string_cstr(name));
    scene->config = tl_string_create(scene->allocator, tl_string_cstr(scene->config));
    tl_scene_init_lua(scene);
    // ==========================================
    // 3. Ler scripts do YAML
    // ==========================================
    const TLString* load_path = tl_config_read_script_path(allocator, scene->config, "load");
    if (load_path != NULL) {
        scene->script_load = tl_string_create(scene->allocator, tl_string_cstr(load_path));
        TLDEBUG("  script.load: %s", tl_string_cstr(scene->script_load));
    } else TLWARN("  - TLScene %s :: script.load is missing", tl_string_cstr(name));

    const TLString* unload_path = tl_config_read_script_path(allocator, scene->config, "unload");
    if (unload_path != NULL) {
        scene->script_unload = tl_string_create(scene->allocator, tl_string_cstr(unload_path));
        TLDEBUG("  script.unload: %s", tl_string_cstr(scene->script_unload));
    } else TLWARN("  - TLScene %s :: script.unload is missing", tl_string_cstr(name));

    const TLString* frame_begin_path = tl_config_read_script_path(allocator, scene->config, "frame_begin");
    if (frame_begin_path != NULL) {
        scene->script_frame_begin = tl_string_create(scene->allocator, tl_string_cstr(frame_begin_path));
        TLDEBUG("  script.frame_begin: %s", tl_string_cstr(scene->script_frame_begin));
    } else TLWARN("  - TLScene %s :: script.frame_begin is missing", tl_string_cstr(name));

    const TLString* frame_end_path = tl_config_read_script_path(allocator, scene->config, "frame_end");
    if (frame_end_path != NULL) {
        scene->script_frame_end = tl_string_create(scene->allocator, tl_string_cstr(frame_end_path));
        TLDEBUG("  script.frame_end: %s", tl_string_cstr(scene->script_frame_end));
    } else TLWARN("  - TLScene %s :: script.frame_end is missing", tl_string_cstr(name));

    tl_memory_allocator_destroy(allocator);
    TL_PROFILER_POP_WITH(scene)
}


#endif