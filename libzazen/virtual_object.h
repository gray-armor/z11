#ifndef LIBZAZEN_VIRTUAL_OBJECT_H
#define LIBZAZEN_VIRTUAL_OBJECT_H

#include <cglm/cglm.h>
#include <wayland-server.h>

#include "compositor.h"

struct zazen_virtual_object {
  struct wl_resource *resource;
  struct wl_signal destroy_signal;
  struct wl_signal commit_signal;
  struct wl_listener component_frame_signal_listener;
  struct wl_list pending_frame_callback_list;
  struct wl_list frame_callback_list;
  mat4 model_matrix;
  struct wl_signal model_matrix_change_signal;
};

struct zazen_virtual_object *zazen_virtual_object_create(
    struct wl_client *client, uint32_t id, struct zazen_compositor *compositor);

void zazen_virtual_object_update_model_matrix(
    struct zazen_virtual_object *virtual_object, mat4 model_matrix);

#endif  //  LIBZAZEN_VIRTUAL_OBJECT_H
