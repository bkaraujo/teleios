#ifndef __TELEIOS_SCENE__
#define __TELEIOS_SCENE__

#include "teleios/defines.h"

b8 tl_scene_initialize(void);
b8 tl_scene_activate(const TLString* name);
TLScene * tl_scene_create(void);
void tl_scene_destroy(const TLScene * scene);
b8 tl_scene_terminate(void);

#endif