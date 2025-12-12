#ifndef __TELEIOS_GRAPHICS__
#define __TELEIOS_GRAPHICS__

#include "teleios/defines.h"

typedef void* (*TLFunctionRNA)(void);
typedef void* (*TLFunctionRWA)(u8 argc, void** argv);
typedef void (*TLFunctionVNA)(void);
typedef void (*TLFunctionVWA)(u8 argc, void** argv);

b8 tl_graphics_initialize(void);
b8 tl_graphics_terminate(void);

#include "teleios/graphics_geometry.inl"
#include "teleios/graphics_shader.inl"

void* tl_graphics_submit_rna(b8 wait, TLFunctionRNA function);
void tl_graphics_submit_vna(b8 wait, TLFunctionVNA function);
void* tl_graphics_submit_rwa(b8 wait, TLFunctionRWA function, u8 argc, void** argv);
void tl_graphics_submit_vwa(b8 wait, TLFunctionVWA function, u8 argc, void** argv);

void tl_graphics_clear(void);
void tl_graphics_update(void);

// ============================================================
// Funções de configuração de estado gráfico
// ============================================================
void tl_graphics_set_clear_color(f32 r, f32 g, f32 b, f32 a);

void tl_graphics_enable_depth(void);
void tl_graphics_disable_depth(void);
void tl_graphics_set_depth_function(const TLString* function);

void tl_graphics_enable_blend(void);
void tl_graphics_disable_blend(void);
void tl_graphics_set_blend_equation(const TLString* equation);
void tl_graphics_set_blend_function(const TLString* source, const TLString* target);

#endif
