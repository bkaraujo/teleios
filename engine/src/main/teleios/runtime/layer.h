#ifndef __TELEIOS_LAYER__
#define __TELEIOS_LAYER__

#include "teleios/defines.h"
#include "teleios/core/types.h"

u8 tl_layer_create(TLString* name);

void tl_layer_set_on_awake(u8 layer, void (*on_awake)(void));
void tl_layer_set_on_update(u8 layer, void (*on_update)(f64 delta));
void tl_layer_set_on_simulate(u8 layer, void (*on_simulate)(f64 step));
void tl_layer_set_on_rest(u8 layer, void (*on_rest)(void));

void tl_layer_push(u8 layer);
void tl_layer_pop(u8 layer);

#endif // __TELEIOS_LAYER__
