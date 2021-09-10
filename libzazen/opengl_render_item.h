#ifndef LIBZAZEN_OPENGL_RENDER_ITEM_H
#define LIBZAZEN_OPENGL_RENDER_ITEM_H

#include <wayland-server.h>
#include <z11-opengl-server-protocol.h>

#include "opengl_render_component_back_state.h"
#include "opengl_render_component_manager.h"

struct zazen_opengl_render_item {
  struct zazen_opengl_render_component_manager* manager;
  bool state_changed;

  void* vertex_buffer_data;
  void* texture_2d_data;

  char* vertex_shader_source;
  char* fragment_shader_source;

  int32_t vertex_buffer_size;
  uint32_t vertex_buffer_stride;

  struct wl_array vertex_input_attributes;

  enum z11_opengl_topology topology;

  struct zazen_opengl_render_component_back_state back_state;
};

struct zazen_opengl_render_item* zazen_opengl_render_item_create(
    struct zazen_opengl_render_component_manager* manager);

void zazen_opengl_render_item_destroy(
    struct zazen_opengl_render_item* render_item);

void zazen_opengl_render_item_commit(
    struct zazen_opengl_render_item* render_item);

#endif  // LIBZAZEN_OPENGL_RENDER_ITEM_H
