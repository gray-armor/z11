#ifndef LIBZAZEN_OPENGL_RENDER_COMPONENT_H
#define LIBZAZEN_OPENGL_RENDER_COMPONENT_H

#include "opengl_render_component_back_state.h"
#include "opengl_render_component_manager.h"
#include "util.h"
#include "virtual_object.h"
#include "z11-opengl-server-protocol.h"

struct zazen_opengl_render_component {
  struct wl_resource* resource;
  struct zazen_opengl_render_component_manager* manager;

  bool state_changed;

  struct wl_listener virtual_object_destroy_signal_listener;
  struct wl_listener virtual_object_commit_signal_listener;

  _Nullable struct zazen_opengl_vertex_buffer* vertex_buffer;
  struct wl_listener vertex_buffer_state_change_listener;
  struct wl_listener vertex_buffer_destroy_listener;

  _Nullable struct zazen_opengl_shader_program* shader_program;
  struct wl_listener shader_program_state_change_listener;
  struct wl_listener shader_program_destroy_listener;

  _Nullable struct zazen_opengl_texture_2d* texture_2d;
  struct wl_listener texture_2d_state_change_listener;
  struct wl_listener texture_2d_destroy_listener;

  struct wl_array vertex_input_attributes;
  enum z11_opengl_topology topology;

  struct zazen_opengl_render_component_back_state back_state;
};

struct zazen_opengl_render_component* zazen_opengl_render_component_create(
    struct wl_client* client, uint32_t id, struct zazen_opengl_render_component_manager* manager,
    struct zazen_virtual_object* virtual_object);

#endif  //  LIBZAZEN_OPENGL_RENDER_COMPONENT_H
