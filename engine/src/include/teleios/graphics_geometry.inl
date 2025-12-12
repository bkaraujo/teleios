#include "teleios/defines.h"

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
