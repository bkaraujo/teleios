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

#endif
