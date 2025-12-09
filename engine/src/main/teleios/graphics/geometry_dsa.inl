#ifndef __TELEIOS_GRAPHICS_GEOMETRY_DSA__
#define __TELEIOS_GRAPHICS_GEOMETRY_DSA__

#include "teleios/teleios.h"
#include "glad/glad.h"

static void tl_graphics_geometry_dsa_create(TLGeometry* geometry) {
    glCreateVertexArrays(1, &geometry->vao);

    glCreateBuffers(1, &geometry->ebo);
    glVertexArrayElementBuffer(geometry->vao, geometry->ebo);

    glCreateBuffers(1, &geometry->vbo);

    u32 current_offset = 0;
    for (u8 i = 0; i < geometry->vbo_att_count; ++i) {
        i32 length = 0;
        u32 type = 0;
        u32 attr_size = 0;

        switch (geometry->vbo_att_nfo[i].type) {
            case TL_BYTE1: length = 1; type = GL_BYTE; attr_size = length * sizeof(i8); break;
            case TL_BYTE2: length = 2; type = GL_BYTE; attr_size = length * sizeof(i8); break;
            case TL_BYTE3: length = 3; type = GL_BYTE; attr_size = length * sizeof(i8); break;
            case TL_BYTE4: length = 4; type = GL_BYTE; attr_size = length * sizeof(i8); break;

            case TL_SHORT1: length = 1; type = GL_SHORT; attr_size = length * sizeof(i16); break;
            case TL_SHORT2: length = 2; type = GL_SHORT; attr_size = length * sizeof(i16); break;
            case TL_SHORT3: length = 3; type = GL_SHORT; attr_size = length * sizeof(i16); break;
            case TL_SHORT4: length = 4; type = GL_SHORT; attr_size = length * sizeof(i16); break;

            case TL_INT1: length = 1; type = GL_INT; attr_size = length * sizeof(i32); break;
            case TL_INT2: length = 2; type = GL_INT; attr_size = length * sizeof(i32); break;
            case TL_INT3: length = 3; type = GL_INT; attr_size = length * sizeof(i32); break;
            case TL_INT4: length = 4; type = GL_INT; attr_size = length * sizeof(i32); break;

            case TL_FLOAT1: length = 1; type = GL_FLOAT; attr_size = length * sizeof(f32); break;
            case TL_FLOAT2: length = 2; type = GL_FLOAT; attr_size = length * sizeof(f32); break;
            case TL_FLOAT3: length = 3; type = GL_FLOAT; attr_size = length * sizeof(f32); break;
            case TL_FLOAT4: length = 4; type = GL_FLOAT; attr_size = length * sizeof(f32); break;

            case TL_DOUBLE1: length = 1; type = GL_DOUBLE; attr_size = length * sizeof(f64); break;
            case TL_DOUBLE2: length = 2; type = GL_DOUBLE; attr_size = length * sizeof(f64); break;
            case TL_DOUBLE3: length = 3; type = GL_DOUBLE; attr_size = length * sizeof(f64); break;
            case TL_DOUBLE4: length = 4; type = GL_DOUBLE; attr_size = length * sizeof(f64); break;

            case TL_UBYTE1: length = 1; type = GL_UNSIGNED_BYTE; attr_size = length * sizeof(i8); break;
            case TL_UBYTE2: length = 2; type = GL_UNSIGNED_BYTE; attr_size = length * sizeof(i8); break;
            case TL_UBYTE3: length = 3; type = GL_UNSIGNED_BYTE; attr_size = length * sizeof(i8); break;
            case TL_UBYTE4: length = 4; type = GL_UNSIGNED_BYTE; attr_size = length * sizeof(i8); break;

            case TL_USHORT1: length = 1; type = GL_UNSIGNED_SHORT; attr_size = length * sizeof(i16); break;
            case TL_USHORT2: length = 2; type = GL_UNSIGNED_SHORT; attr_size = length * sizeof(i16); break;
            case TL_USHORT3: length = 3; type = GL_UNSIGNED_SHORT; attr_size = length * sizeof(i16); break;
            case TL_USHORT4: length = 4; type = GL_UNSIGNED_SHORT; attr_size = length * sizeof(i16); break;

            case TL_UINT1: length = 1; type = GL_UNSIGNED_INT; attr_size = length * sizeof(i32); break;
            case TL_UINT2: length = 2; type = GL_UNSIGNED_INT; attr_size = length * sizeof(i32); break;
            case TL_UINT3: length = 3; type = GL_UNSIGNED_INT; attr_size = length * sizeof(i32); break;
            case TL_UINT4: length = 4; type = GL_UNSIGNED_INT; attr_size = length * sizeof(i32); break;
        }

        glEnableVertexArrayAttrib(geometry->vao, i);
        glVertexArrayAttribBinding(geometry->vao, i, 0);
        glVertexArrayAttribFormat(geometry->vao, i, length, type, GL_FALSE, current_offset);

        current_offset += attr_size;
    }

    geometry->vbo_stride = current_offset;
    glVertexArrayVertexBuffer(geometry->vao, 0, geometry->vbo, 0, geometry->vbo_stride);
}

static void tl_graphics_geometry_dsa_upload_indices(TLGeometry* geometry, const u8 count, const u32* indices) {
    const u32 required_size = count * sizeof(u32);

    if (required_size > geometry->ebo_size) {
        glNamedBufferData(geometry->ebo, required_size, indices, GL_STATIC_DRAW);
        geometry->ebo_size = required_size;
    } else {
        glNamedBufferSubData(geometry->ebo, 0, required_size, indices);
    }
}

static void tl_graphics_geometry_dsa_upload_vertices(TLGeometry* geometry, const u8 count, const f32* vertices) {
    const u32 required_size = count * sizeof(f32);

    if (required_size > geometry->vbo_size) {
        glNamedBufferData(geometry->vbo, required_size, vertices, GL_STATIC_DRAW);
        geometry->vbo_size = required_size;
    } else {
        glNamedBufferSubData(geometry->vbo, 0, required_size, vertices);
    }
}

#endif