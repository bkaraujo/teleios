#ifndef __TELEIOS_SCENE__
#define __TELEIOS_SCENE__

#include "teleios/defines.h"

void tl_scene_load(void);
void tl_scene_step(f64 step);
void tl_scene_update(f64 delta);

void tl_scene_frame_begin(void);
void tl_scene_frame_end(void);

#endif