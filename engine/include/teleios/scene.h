#ifndef __TELEIOS_SCENE__
#define __TELEIOS_SCENE__

#include "teleios/defines.h"

// Gerenciamento do módulo
b8 tl_scene_initialize(void);
b8 tl_scene_terminate(void);

// Gerenciamento de cenas
b8 tl_scene_activate(const TLString* name);
TLScene* tl_scene_create(void);
void tl_scene_destroy(const TLScene* scene);

// ============================================================
// SCRIPTS DO CLIENTE (fornecidos via YAML - OBRIGATÓRIOS)
// ============================================================
void tl_scene_load(TLScene* scene);        // Executa script_load
void tl_scene_unload(TLScene* scene);      // Executa script_unload
void tl_scene_frame_begin(TLScene* scene); // Executa script_frame_begin (APÓS engine)
void tl_scene_frame_end(TLScene* scene);   // Executa script_frame_end (ANTES swap)

// ============================================================
// IMPLEMENTAÇÕES FIXAS DO ENGINE (futuro)
// ============================================================
void tl_scene_step(f64 step);   // Física (implementação fixa)
void tl_scene_update(f64 delta); // Update (implementação fixa)

#endif