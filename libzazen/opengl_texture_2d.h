#ifndef LIBZAZEN_OPENGL_TEXTURE_2D_H
#define LIBZAZEN_OPENGL_TEXTURE_2D_H

#include <wayland-server.h>

#include "util.h"
#include "z11-opengl-server-protocol.h"

struct zazen_opengl_texture_2d_state {
  _NonNull struct wl_resource* raw_buffer_resource;
  enum z11_opengl_texture_2d_format format;
  int32_t width;
  int32_t height;
};

struct zazen_opengl_texture_2d {
  _Nullable struct zazen_opengl_texture_2d_state* state;
  struct wl_listener raw_buffer_destroy_listener;
  struct wl_signal destroy_signal;
  struct wl_signal state_change_signal;
};

struct zazen_opengl_texture_2d* zazen_opengl_texture_2d_create(struct wl_client* client, uint32_t id);

#endif  //  LIBZAZEN_OPENGL_TEXTURE_2D_
