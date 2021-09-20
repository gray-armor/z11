#ifndef LIBZAZEN_RAY_CLIENT_H
#define LIBZAZEN_RAY_CLIENT_H

#include <wayland-server.h>

#include "ray.h"

struct zazen_ray_client {
  struct wl_list link;
  struct wl_resource* resource;

  struct wl_listener ray_destroy_signal_listener;
};

struct zazen_ray_client* zazen_ray_client_create(struct zazen_ray* ray,
                                                 struct wl_client* client,
                                                 uint32_t id);

#endif  // LIBZAZEN_RAY_CLIENT_H
