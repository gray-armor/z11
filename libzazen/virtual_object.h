#ifndef LIBZAZEN_VIRTUAL_OBJECT_H
#define LIBZAZEN_VIRTUAL_OBJECT_H

#include <wayland-server.h>

#include "compositor.h"

struct zazen_virtual_object {
  struct wl_resource *resource;
  struct wl_signal destroy_signal;
  struct wl_signal commit_signal;
  struct wl_listener component_frame_signal_listener;
  struct wl_list pending_frame_callback_list;
  struct wl_list frame_callback_list;
};

struct zazen_virtual_object *zazen_virtual_object_create(
    struct wl_client *client, uint32_t id, struct zazen_compositor *compositor);

#endif  //  LIBZAZEN_VIRTUAL_OBJECT_H
