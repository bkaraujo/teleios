#ifndef __TELEIOS_SCENE__
#define __TELEIOS_SCENE__

#include "teleios/defines.h"

// ######################################
// Gerenciamento do módulo
// ######################################
b8 tl_scene_initialize(void);
b8 tl_scene_terminate(void);
// ######################################
// Gerenciamento de cenas
// ######################################
TLScene* tl_scene_create(void);
b8 tl_scene_activate(const TLString* name);
void tl_scene_load(TLScene* scene);
void tl_scene_unload(TLScene* scene);
void tl_scene_destroy(const TLScene* scene);
// ######################################
// Processamento da cena ativa
// ######################################
void tl_scene_frame_begin();
void tl_scene_frame_end();
void tl_scene_step(f64 step);
void tl_scene_update(f64 delta);

#endif