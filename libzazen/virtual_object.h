#ifndef LIBZAZEN_VIRTUAL_OBJECT_H
#define LIBZAZEN_VIRTUAL_OBJECT_H

#include <wayland-server.h>

struct zazen_virtual_object {
  struct wl_resource *resource;
  struct wl_signal destroy_signal;
  struct wl_signal commit_signal;
};

struct zazen_virtual_object *zazen_virtual_object_create(struct wl_client *client, uint32_t id);

#endif  //  LIBZAZEN_VIRTUAL_OBJECT_H
