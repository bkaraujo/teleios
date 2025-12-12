#ifndef __TELEIOS_GRAPHICS__
#define __TELEIOS_GRAPHICS__

#include "teleios/defines.h"

typedef void* (*TLFunctionRNA)(void);
typedef void* (*TLFunctionRWA)(u8 argc, void** argv);
typedef void (*TLFunctionVNA)(void);
typedef void (*TLFunctionVWA)(u8 argc, void** argv);

b8 tl_graphics_initialize(void);
b8 tl_graphics_terminate(void);

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
// ============================================================
// Queue API
// ============================================================
void* tl_graphics_submit_rna(b8 wait, TLFunctionRNA function);
void tl_graphics_submit_vna(b8 wait, TLFunctionVNA function);
void* tl_graphics_submit_rwa(b8 wait, TLFunctionRWA function, u8 argc, void** argv);
void tl_graphics_submit_vwa(b8 wait, TLFunctionVWA function, u8 argc, void** argv);
// ============================================================
// Shader API
// ============================================================
u32 tl_graphics_shader_load(const TLString* path);
void tl_graphics_shader_unload(u32 shader);
// ============================================================
// Geometry API
// ============================================================
typedef struct {
    TLString* name;
    u8 stride;
} TLGeometryIndice;

typedef enum {
    TL_BYTE1, TL_BYTE2, TL_BYTE3, TL_BYTE4,
    TL_SHORT1, TL_SHORT2, TL_SHORT3, TL_SHORT4,
    TL_INT1, TL_INT2, TL_INT3, TL_INT4,
    TL_FLOAT1, TL_FLOAT2, TL_FLOAT3, TL_FLOAT4,
    TL_DOUBLE1, TL_DOUBLE2, TL_DOUBLE3, TL_DOUBLE4,

    TL_UBYTE1, TL_UBYTE2, TL_UBYTE3, TL_UBYTE4,
    TL_USHORT1, TL_USHORT2, TL_USHORT3, TL_USHORT4,
    TL_UINT1, TL_UINT2, TL_UINT3, TL_UINT4,
} TLAttributeTypes;

typedef struct {
    TLString* name;
    TLAttributeTypes type;
} TLGeometryAttribute;

typedef struct {
    u32 usage;          // Static/Dynamic Geometry
    TLArray* indices;   // Multiple indexing geometry
    TLArray* buffers;   // Multiple region buffer
} TLGeometrySpec;

typedef struct TLGeometry TLGeometry;

void tl_graphics_geometry_bind(const TLGeometry* geometry);

/** Creates a Vertex Array in the GPU */
TLGeometry* tl_graphics_geometry_create(TLAllocator* allocator, u8 attribute_count, const TLGeometryAttribute* attributes);
void tl_graphics_geometry_destroy(TLGeometry* geometry);

/** Upload the indices to the GPU */
void tl_graphics_geometry_upload_indices(TLGeometry* geometry, u8 count, const u32* indices);

/** Upload the vertices to the GPU */
void tl_graphics_geometry_upload_vertices(TLGeometry* geometry, u8 count, const f32* vertices);

#endif
