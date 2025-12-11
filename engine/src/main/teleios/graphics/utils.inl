#ifndef __TELEIOS_GRAPHICS_UTILS__
#define __TELEIOS_GRAPHICS_UTILS__

#include "teleios/teleios.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// #####################################################################################################################
// tl_graphics_clear
// #####################################################################################################################
static void tl_renderer_clear(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void tl_graphics_clear(void) {
    tl_graphics_submit_vna(false, tl_renderer_clear);
}

// #####################################################################################################################
// tl_graphics_update
// #####################################################################################################################
static void tl_renderer_swap(void) {
    glfwSwapBuffers(tl_window_handler());
    global->frame_count++;
}

void tl_graphics_update(void) {
    tl_graphics_submit_vna(false, tl_renderer_swap);
}

// #####################################################################################################################
// tl_graphics_set_clear_color
// #####################################################################################################################
static f32 m_clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};

static void tl_renderer_set_clear_color(void) {
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
}

void tl_graphics_set_clear_color(f32 r, f32 g, f32 b, f32 a) {
    m_clear_color[0] = r;
    m_clear_color[1] = g;
    m_clear_color[2] = b;
    m_clear_color[3] = a;
    tl_graphics_submit_vna(true, tl_renderer_set_clear_color);
}

// #####################################################################################################################
// tl_graphics_enable_depth / tl_graphics_disable_depth
// #####################################################################################################################
static void tl_renderer_enable_depth(void) {
    glEnable(GL_DEPTH_TEST);
}

static void tl_renderer_disable_depth(void) {
    glDisable(GL_DEPTH_TEST);
}

void tl_graphics_enable_depth(void) {
    tl_graphics_submit_vna(true, tl_renderer_enable_depth);
}

void tl_graphics_disable_depth(void) {
    tl_graphics_submit_vna(true, tl_renderer_disable_depth);
}

// #####################################################################################################################
// tl_graphics_set_depth_function
// #####################################################################################################################
static GLenum m_depth_function = GL_LESS;

static GLenum tl_graphics_parse_depth_function(const TLString* function) {
    if (tl_string_equals_cstr(function, "NEVER"))    return GL_NEVER;
    if (tl_string_equals_cstr(function, "LESS"))     return GL_LESS;
    if (tl_string_equals_cstr(function, "EQUAL"))    return GL_EQUAL;
    if (tl_string_equals_cstr(function, "LEQUAL"))   return GL_LEQUAL;
    if (tl_string_equals_cstr(function, "GREATER"))  return GL_GREATER;
    if (tl_string_equals_cstr(function, "NOTEQUAL")) return GL_NOTEQUAL;
    if (tl_string_equals_cstr(function, "GEQUAL"))   return GL_GEQUAL;
    if (tl_string_equals_cstr(function, "ALWAYS"))   return GL_ALWAYS;
    return GL_LESS;  // Default
}

static void tl_renderer_set_depth_function(void) {
    glDepthFunc(m_depth_function);
}

void tl_graphics_set_depth_function(const TLString* function) {
    m_depth_function = tl_graphics_parse_depth_function(function);
    tl_graphics_submit_vna(true, tl_renderer_set_depth_function);
}

// #####################################################################################################################
// tl_graphics_enable_blend / tl_graphics_disable_blend
// #####################################################################################################################
static void tl_renderer_enable_blend(void) {
    glEnable(GL_BLEND);
}

static void tl_renderer_disable_blend(void) {
    glDisable(GL_BLEND);
}

void tl_graphics_enable_blend(void) {
    tl_graphics_submit_vna(true, tl_renderer_enable_blend);
}

void tl_graphics_disable_blend(void) {
    tl_graphics_submit_vna(true, tl_renderer_disable_blend);
}

// #####################################################################################################################
// tl_graphics_set_blend_equation
// #####################################################################################################################
static GLenum m_blend_equation = GL_FUNC_ADD;

static GLenum tl_graphics_parse_blend_equation(const TLString* equation) {
    if (tl_string_equals_cstr(equation, "FUNC_ADD"))              return GL_FUNC_ADD;
    if (tl_string_equals_cstr(equation, "FUNC_SUBTRACT"))         return GL_FUNC_SUBTRACT;
    if (tl_string_equals_cstr(equation, "FUNC_REVERSE_SUBTRACT")) return GL_FUNC_REVERSE_SUBTRACT;
    if (tl_string_equals_cstr(equation, "MIN"))                   return GL_MIN;
    if (tl_string_equals_cstr(equation, "MAX"))                   return GL_MAX;
    return GL_FUNC_ADD;  // Default
}

static void tl_renderer_set_blend_equation(void) {
    glBlendEquation(m_blend_equation);
}

void tl_graphics_set_blend_equation(const TLString* equation) {
    m_blend_equation = tl_graphics_parse_blend_equation(equation);
    tl_graphics_submit_vna(true, tl_renderer_set_blend_equation);
}

// #####################################################################################################################
// tl_graphics_set_blend_function
// #####################################################################################################################
static GLenum m_blend_src = GL_SRC_ALPHA;
static GLenum m_blend_dst = GL_ONE_MINUS_SRC_ALPHA;

static GLenum tl_graphics_parse_blend_factor(const TLString* factor) {
    if (tl_string_equals_cstr(factor, "ZERO"))                     return GL_ZERO;
    if (tl_string_equals_cstr(factor, "ONE"))                      return GL_ONE;
    if (tl_string_equals_cstr(factor, "SRC_COLOR"))                return GL_SRC_COLOR;
    if (tl_string_equals_cstr(factor, "ONE_MINUS_SRC_COLOR"))      return GL_ONE_MINUS_SRC_COLOR;
    if (tl_string_equals_cstr(factor, "DST_COLOR"))                return GL_DST_COLOR;
    if (tl_string_equals_cstr(factor, "ONE_MINUS_DST_COLOR"))      return GL_ONE_MINUS_DST_COLOR;
    if (tl_string_equals_cstr(factor, "SRC_ALPHA"))                return GL_SRC_ALPHA;
    if (tl_string_equals_cstr(factor, "ONE_MINUS_SRC_ALPHA"))      return GL_ONE_MINUS_SRC_ALPHA;
    if (tl_string_equals_cstr(factor, "DST_ALPHA"))                return GL_DST_ALPHA;
    if (tl_string_equals_cstr(factor, "ONE_MINUS_DST_ALPHA"))      return GL_ONE_MINUS_DST_ALPHA;
    if (tl_string_equals_cstr(factor, "CONSTANT_COLOR"))           return GL_CONSTANT_COLOR;
    if (tl_string_equals_cstr(factor, "ONE_MINUS_CONSTANT_COLOR")) return GL_ONE_MINUS_CONSTANT_COLOR;
    if (tl_string_equals_cstr(factor, "CONSTANT_ALPHA"))           return GL_CONSTANT_ALPHA;
    if (tl_string_equals_cstr(factor, "ONE_MINUS_CONSTANT_ALPHA")) return GL_ONE_MINUS_CONSTANT_ALPHA;
    return GL_ONE;  // Default
}

static void tl_renderer_set_blend_function(void) {
    glBlendFunc(m_blend_src, m_blend_dst);
}

void tl_graphics_set_blend_function(const TLString* source, const TLString* target) {
    m_blend_src = tl_graphics_parse_blend_factor(source);
    m_blend_dst = tl_graphics_parse_blend_factor(target);
    tl_graphics_submit_vna(true, tl_renderer_set_blend_function);
}

#endif