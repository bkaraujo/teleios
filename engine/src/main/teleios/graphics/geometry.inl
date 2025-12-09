#ifndef __TELEIOS_GRAPHICS_GEOMETRY__
#define __TELEIOS_GRAPHICS_GEOMETRY__

#include "teleios/teleios.h"
#include "glad/glad.h"

struct TLGeometry {
    u32 vao;

    u32 ebo;
    u32 ebo_size;

    u32 vbo;
    u32 vbo_size;
    u32 vbo_stride;
    u8 vbo_att_count;
    TLGeometryAttribute* vbo_att_nfo;

    TLAllocator* allocator;
} ;

#include "teleios/graphics/geometry_dsa.inl"
#include "teleios/graphics/geometry_ndsa.inl"

// ---------------------------------
// Public API
// ---------------------------------

void tl_graphics_geometry_bind(const TLGeometry* geometry) {
    glBindVertexArray(geometry->vao);
}

TLGeometry* tl_graphics_geometry_create(TLAllocator* allocator, u8 attribute_count, const TLGeometryAttribute* attributes) {
    TLGeometry* geometry = tl_memory_alloc(allocator, TL_MEMORY_GRAPHICS, sizeof(TLGeometry));
    geometry->vbo_att_count = attribute_count;
    geometry->ebo_size = 0;
    geometry->vbo_size = 0;
    geometry->allocator = allocator;

    // Allocate and copy attributes array
    geometry->vbo_att_nfo = tl_memory_alloc(allocator, TL_MEMORY_GRAPHICS, sizeof(TLGeometryAttribute) * attribute_count);
    for (u8 i = 0; i < attribute_count; i++) {
        geometry->vbo_att_nfo[i].type = attributes[i].type;
        // Copy string name if present
        if (attributes[i].name) {
            geometry->vbo_att_nfo[i].name = tl_string_copy(attributes[i].name);
        } else {
            geometry->vbo_att_nfo[i].name = NULL;
        }
    }

    if (GLVersion.major >= 4 && GLVersion.minor >= 5)   { tl_graphics_geometry_dsa_create(geometry); }
    else                                                { tl_graphics_geometry_ndsa_create(geometry); }

    return geometry;
}

void tl_graphics_geometry_destroy(TLGeometry* geometry) {
    // Delete OpenGL objects
    if (geometry->vao) {
        glDeleteVertexArrays(1, &geometry->vao);
    }
    if (geometry->vbo) {
        glDeleteBuffers(1, &geometry->vbo);
    }
    if (geometry->ebo) {
        glDeleteBuffers(1, &geometry->ebo);
    }

    // Free attribute names
    for (u8 i = 0 ; i < geometry->vbo_att_count; i++) {
        if (geometry->vbo_att_nfo[i].name) {
            tl_string_destroy(geometry->vbo_att_nfo[i].name);
        }
    }

    // Free attributes array
    tl_memory_free(geometry->allocator, geometry->vbo_att_nfo);

    // Free geometry structure
    tl_memory_free(geometry->allocator, geometry);
}

void tl_graphics_geometry_upload_indices(TLGeometry* geometry, const u8 count, const u32* indices) {
    if (GLVersion.major >= 4 && GLVersion.minor >= 5) {
        tl_graphics_geometry_dsa_upload_indices(geometry, count, indices);
        return;
    }

    tl_graphics_geometry_bind(geometry);
    tl_graphics_geometry_ndsa_upload_indices(geometry, count, indices);
}

void tl_graphics_geometry_upload_vertices(TLGeometry* geometry, const u8 count, const f32* vertices) {
    if (GLVersion.major >= 4 && GLVersion.minor >= 5) {
        tl_graphics_geometry_dsa_upload_vertices(geometry, count, vertices);
        return;
    }

    tl_graphics_geometry_bind(geometry);
    tl_graphics_geometry_ndsa_upload_vertices(geometry, count, vertices);
}

#endif
