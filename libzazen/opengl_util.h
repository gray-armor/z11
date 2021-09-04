#ifndef LIBZAZEN_OPENGL_UTIL_H
#define LIBZAZEN_OPENGL_UTIL_H

#include <GL/glew.h>
#include <libzazen.h>

#include "z11-opengl-server-protocol.h"

struct gl_vertex_input_attribute {
  uint32_t location;
  enum z11_opengl_vertex_input_attribute_format format;
  uint32_t offset;
};

void gl_commit_texture_2d(struct zazen_opengl_render_component_back_state* back_state,
                          enum z11_opengl_texture_2d_format format, int32_t width, int32_t height, void* data,
                          int32_t buffer_size);

bool gl_commit_shader_program(struct zazen_opengl_render_component_back_state* back_state,
                              const char* vertex_shader_source, const char* fragment_shader_source);

void gl_commit_vertex_buffer(struct zazen_opengl_render_component_back_state* back_state, int32_t buffer_size,
                             void* data, uint32_t stride);

void gl_commit_topology_mode(struct zazen_opengl_render_component_back_state* back_state,
                             enum z11_opengl_topology topology);

void gl_commit_vertex_array(struct zazen_opengl_render_component_back_state* back_state,
                            struct wl_array* vertex_input_attributes);

#endif  // LIBZAZEN_OPENGL_UTIL_H
