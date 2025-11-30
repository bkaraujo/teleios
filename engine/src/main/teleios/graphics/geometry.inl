#ifndef __TELEIOS_GRAPHICS_GEOMETRY__
#define __TELEIOS_GRAPHICS_GEOMETRY__

#include "teleios/teleios.h"
#include "glad/glad.h"

static void tl_graphics_geometry_dsa_create(TLGeometry* geometry);
static void tl_graphics_geometry_dsa_upload_indices(const TLGeometry* geometry, u8 count, const i32* indices);
static void tl_graphics_geometry_dsa_upload_vertices(const TLGeometry* geometry, u8 count, const f32* vertices);

static void tl_graphics_geometry_ndsa_create(TLGeometry* geometry);
static void tl_graphics_geometry_ndsa_upload_indices(const TLGeometry* geometry, u8 count, const i32* indices);
static void tl_graphics_geometry_ndsa_upload_vertices(const TLGeometry* geometry, u8 count, const f32* vertices);

void tl_graphics_geometry_bind(const TLGeometry* geometry) {
    glBindVertexArray(geometry->vao);
}

void tl_graphics_geometry_create(TLGeometry* geometry) {
    if (GLVersion.major >= 4 && GLVersion.minor >= 5) {
        tl_graphics_geometry_dsa_create(geometry);
        return;
    }

    tl_graphics_geometry_ndsa_create(geometry);
}


void tl_graphics_geometry_upload_indices(const TLGeometry* geometry, const u8 count, const i32* indices) {
    if (GLVersion.major >= 4 && GLVersion.minor >= 5) {
        tl_graphics_geometry_dsa_upload_indices(geometry, count, indices);
        return;
    }

    tl_graphics_geometry_bind(geometry);
    tl_graphics_geometry_ndsa_upload_indices(geometry, count, indices);
}

void tl_graphics_geometry_upload_vertices(const TLGeometry* geometry, const u8 count, const f32* vertices) {
    if (GLVersion.major >= 4 && GLVersion.minor >= 5) {
        tl_graphics_geometry_dsa_upload_vertices(geometry, count, vertices);
        return;
    }

    tl_graphics_geometry_bind(geometry);
    tl_graphics_geometry_ndsa_upload_vertices(geometry, count, vertices);
}

// ---------------------------------
// Direct State Access
// ---------------------------------

static void tl_graphics_geometry_dsa_create(TLGeometry* geometry) {
    glCreateVertexArrays(1, &geometry->vao);
    glCreateBuffers(1, &geometry->ibo);
    glCreateBuffers(1, &geometry->vbo);
}

static void tl_graphics_geometry_dsa_upload_indices(const TLGeometry* geometry, const u8 count, const i32* indices) {
    glNamedBufferData(geometry->ibo, count * sizeof(i32), indices, GL_STATIC_DRAW);
    glVertexArrayElementBuffer(geometry->vao, geometry->ibo);
}

static void tl_graphics_geometry_dsa_upload_vertices(const TLGeometry* geometry, const u8 count, const f32* vertices) {
    glNamedBufferData(geometry->vbo, count * sizeof(f32), vertices, GL_STATIC_DRAW);

    // Bind VBO to binding point 0
    glVertexArrayVertexBuffer(geometry->vao, 0, geometry->vbo, 0, sizeof(f32) * 3);

    // Enable and configure vertex attribute 0 (position)
    glEnableVertexArrayAttrib(geometry->vao, 0);
    glVertexArrayAttribFormat(geometry->vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(geometry->vao, 0, 0);
}

// ---------------------------------
// State Machine
// ---------------------------------

static void tl_graphics_geometry_ndsa_create(TLGeometry* geometry) {
    glGenVertexArrays(1, &geometry->vao);
    glGenBuffers(1, &geometry->ibo);
    glGenBuffers(1, &geometry->vbo);
}

static void tl_graphics_geometry_ndsa_upload_indices(const TLGeometry* geometry, const u8 count, const i32* indices) {
    u32 ebo; glGenBuffers(1, &ebo);

    glBindVertexArray(geometry->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(f32), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

static void tl_graphics_geometry_ndsa_upload_vertices(const TLGeometry* geometry, const u8 count, const f32* vertices) {
    u32 vbo; glGenBuffers(1, &vbo);

    glBindVertexArray(geometry->vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(f32), vertices, GL_STATIC_DRAW);

    // Configure vertex attribute 0 (position: 3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 3, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#endif