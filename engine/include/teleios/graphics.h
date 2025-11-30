#ifndef __TELEIOS_GRAPHICS__
#define __TELEIOS_GRAPHICS__

#include "teleios/defines.h"

typedef void* (*TLFunctionRNA)(void);
typedef void* (*TLFunctionRWA)(u8 argc, void** argv);
typedef void (*TLFunctionVNA)(void);
typedef void (*TLFunctionVWA)(u8 argc, void** argv);

b8 tl_graphics_initialize(void);
b8 tl_graphics_terminate(void);

u32 tl_graphics_shader_load(const TLString* path);

typedef struct {
    TLString* name;
    u8 stride;
} TLGeometryIndice;

typedef enum {
    TL_BYTE1, TL_BYTE2, TL_BYTE3, TL_BYTE4,
    TL_SHORT1, TL_SHORT2, TL_SHORT3, TL_SHORT4,
    TL_INT1, TL_INT2, TL_INT3, TL_INT4,
    TL_FLOAT1, TL_FLOAT2, TL_FLOAT3, TL_FLOAT4,
    TL_DOUBLE1, TL_DOUBLE2, TL_DOUBLE3, TL_DOUBLE4
} TLBufferTypes;

typedef struct {
    TLString* name;
    TLBufferTypes type;
} TLGeometryBuffer;

typedef struct {
    u32 usage;          // Static/Dynamic Geometry
    TLArray* indices;   // Multiple indexing geometry
    TLArray* buffers;   // Multiple region buffer
} TLGeometrySpec;

typedef struct {
    u32 vao;
    u32 ibo;
    u32 vbo;
} TLGeometry;

void tl_graphics_geometry_bind(const TLGeometry* geometry);

/** Creates a Vertex Array in the GPU */
void tl_graphics_geometry_create(TLGeometry* geometry);

/** Upload the indices to the GPU */
void tl_graphics_geometry_upload_indices(const TLGeometry* geometry, u8 count, const i32* indices);

/** Upload the vertices to the GPU */
void tl_graphics_geometry_upload_vertices(const TLGeometry* geometry, u8 count, const f32* vertices);

void* tl_graphics_submit_rna(b8 wait, TLFunctionRNA function);
void tl_graphics_submit_vna(b8 wait, TLFunctionVNA function);
void* tl_graphics_submit_rwa(b8 wait, TLFunctionRWA function, u8 argc, void** argv);
void tl_graphics_submit_vwa(b8 wait, TLFunctionVWA function, u8 argc, void** argv);

#endif
