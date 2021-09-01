#ifndef LIBZAZEN_OPENGL_FOR_RENDER_STATE_H
#define LIBZAZEN_OPENGL_FOR_RENDER_STATE_H

#include <GL/glew.h>
#include <libzazen.h>
#include <wayland-server.h>

#include "opengl_render_component_manager.h"

struct zazen_opengl_render_item {
  struct zazen_opengl_render_component_manager* manager;
  bool state_changed;

  void* vertex_buffer_data;
  void* texture_2d_data;

  char* vertex_shader_source;
  char* fragment_shader_source;

  uint32_t vertex_location;
  GLuint vertex_size;
  GLenum vertex_type;
  uint32_t vertex_offset;

  struct zazen_opengl_render_component_back_state back_state;
};

struct zazen_opengl_render_item* zazen_opengl_render_item_create(
    struct zazen_opengl_render_component_manager* manager);

void zazen_opengl_render_item_commit(struct zazen_opengl_render_item* render_item);

#endif  // LIBZAZEN_OPENGL_FOR_RENDER_STATE_H
