#ifndef LIBZAZEN_OPENGL_VERTEX_BUFFER_H
#define LIBZAZEN_OPENGL_VERTEX_BUFFER_H

#include <wayland-server.h>

#include "util.h"

struct zazen_opengl_vertex_buffer {
  _Nullable struct wl_resource *raw_buffer_resource;
  uint32_t stride;
  struct wl_listener raw_buffer_resource_destroy_listener;
  struct wl_signal destroy_signal;
  struct wl_signal state_change_signal;
};

struct zazen_opengl_vertex_buffer *zazen_opengl_vertex_buffer_create(struct wl_client *client, uint32_t id);

#endif  //  LIBZAZEN_OPENGL_VERTEX_BUFFER_H
