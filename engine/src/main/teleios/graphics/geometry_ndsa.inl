#ifndef __TELEIOS_GRAPHICS_GEOMETRY_NDSA__
#define __TELEIOS_GRAPHICS_GEOMETRY_NDSA__

#include "teleios/teleios.h"
#include "glad/glad.h"

static void tl_graphics_geometry_ndsa_create(TLGeometry* geometry) {
    glGenVertexArrays(1, &geometry->vao);
    glBindVertexArray(geometry->vao);

    glGenBuffers(1, &geometry->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->ebo);

    glGenBuffers(1, &geometry->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);

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

        current_offset += attr_size;
    }

    geometry->vbo_stride = current_offset;

    current_offset = 0;
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

        glVertexAttribPointer(i, length, type, GL_FALSE, (i32) geometry->vbo_stride, (void*)(uintptr_t) current_offset);
        glEnableVertexAttribArray(i);

        current_offset += attr_size;
    }
}

static void tl_graphics_geometry_ndsa_upload_indices(TLGeometry* geometry, const u8 count, const u32* indices) {
    const u32 required_size = count * sizeof(u32);

    glBindVertexArray(geometry->vao);

    if (required_size > geometry->ebo_size) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, required_size, indices, GL_STATIC_DRAW);
        geometry->ebo_size = required_size;
    } else {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, required_size, indices);
    }
}

static void tl_graphics_geometry_ndsa_upload_vertices(TLGeometry* geometry, const u8 count, const f32* vertices) {
    const u32 required_size = count * sizeof(f32);

    glBindVertexArray(geometry->vao);

    if (required_size > geometry->vbo_size) {
        glBufferData(GL_ARRAY_BUFFER, required_size, vertices, GL_STATIC_DRAW);
        geometry->vbo_size = required_size;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, required_size, vertices);
    }
}

#endif