#ifndef __TELEIOS_GRAPHICS_GEOMETRY_DSA__
#define __TELEIOS_GRAPHICS_GEOMETRY_DSA__

#include "teleios/teleios.h"
#include "glad/glad.h"

#define TL_CREATE_CASE(tlName, glName, cType) \
case TL_##tlName##1: length = 1; type = GL_##glName; attr_size = length * sizeof(cType); break; \
case TL_##tlName##2: length = 2; type = GL_##glName; attr_size = length * sizeof(cType); break; \
case TL_##tlName##3: length = 3; type = GL_##glName; attr_size = length * sizeof(cType); break; \
case TL_##tlName##4: length = 4; type = GL_##glName; attr_size = length * sizeof(cType); break;

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
            // Tipos Signed (Nomes batem)
            TL_CREATE_CASE(BYTE,  BYTE,  i8)
            TL_CREATE_CASE(SHORT, SHORT, i16)
            TL_CREATE_CASE(INT,   INT,   i32)
            TL_CREATE_CASE(FLOAT, FLOAT, f32)
            TL_CREATE_CASE(DOUBLE, DOUBLE, f64)

            // Tipos Unsigned (Nomes OpenGL são diferentes)
            TL_CREATE_CASE(UBYTE,  UNSIGNED_BYTE,  u8)
            TL_CREATE_CASE(USHORT, UNSIGNED_SHORT, u16)
            TL_CREATE_CASE(UINT,   UNSIGNED_INT,   u32)
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