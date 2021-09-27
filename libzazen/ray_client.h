#ifndef LIBZAZEN_RAY_CLIENT_H
#define LIBZAZEN_RAY_CLIENT_H

#include <wayland-server.h>

#include "ray.h"

struct zazen_ray_client {
  struct wl_list link;
  struct wl_client* client;
  struct wl_list ray_resources;

  struct wl_listener client_destroy_listener;
  struct wl_listener ray_destroy_signal_listener;
};

struct zazen_ray_client* zazen_ray_client_find_or_create(
    struct zazen_ray* ray, struct wl_client* client);

void zazen_ray_client_add_resource(struct zazen_ray_client* ray_client,
                                   struct wl_resource* resource);

#endif  // LIBZAZEN_RAY_CLIENT_H
