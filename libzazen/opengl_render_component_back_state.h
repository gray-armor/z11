#ifndef LIBZAZEN_OPENGL_RENDER_COMPONENT_BACK_STATE_H
#define LIBZAZEN_OPENGL_RENDER_COMPONENT_BACK_STATE_H

#include <GL/glew.h>
#include <libzazen.h>

#include "z11-opengl-server-protocol.h"

struct zazen_opengl_render_component_back_state_vertex_input_attribute {
  uint32_t location;
  enum z11_opengl_vertex_input_attribute_format format;
  uint32_t offset;
};

// texture
void zazen_opengl_render_component_back_state_delete_texture_2d(
    struct zazen_opengl_render_component_back_state* back_state);

void zazen_opengl_render_component_back_state_generate_texture_2d(
    struct zazen_opengl_render_component_back_state* back_state,
    enum z11_opengl_texture_2d_format format, int32_t width, int32_t height,
    void* data, int32_t buffer_size);

// shader
void zazen_opengl_render_component_back_state_delete_shader_program(
    struct zazen_opengl_render_component_back_state* back_state);

bool zazen_opengl_render_component_back_state_generate_shader_program(
    struct zazen_opengl_render_component_back_state* back_state,
    const char* vertex_shader_source, const char* fragment_shader_source);

// vertex buffer
void zazen_opengl_render_component_back_state_delete_vertex_buffer(
    struct zazen_opengl_render_component_back_state* back_state);

void zazen_opengl_render_component_back_state_generate_vertex_buffer(
    struct zazen_opengl_render_component_back_state* back_state,
    int32_t buffer_size, void* data, uint32_t stride);

// topology mode
void zazen_opengl_render_component_back_state_set_topology_mode(
    struct zazen_opengl_render_component_back_state* back_state,
    enum z11_opengl_topology topology);

// vertex_array
void zazen_opengl_render_component_back_state_delete_vertex_array(
    struct zazen_opengl_render_component_back_state* back_state);

void zazen_opengl_render_component_back_state_generate_vertex_array(
    struct zazen_opengl_render_component_back_state* back_state,
    struct wl_array* vertex_input_attributes);

void zazen_opengl_render_component_back_state_destroy(
    struct zazen_opengl_render_component_back_state* back_state);

#endif  // LIBZAZEN_OPENGL_RENDER_COMPONENT_BACK_STATE_H
