#ifndef LIBZAZEN_DATA_SOURCE_H
#define LIBZAZEN_DATA_SOURCE_H

#include <wayland-server.h>

struct zazen_data_source {
  struct wl_client* client;
  struct wl_resource* resource;

  struct wl_listener client_destroy_listener;
};

struct zazen_data_source* zazen_data_source_create(struct wl_client* client,
                                                   struct wl_resource* resource,
                                                   uint32_t id);

#endif  // LIBZAZEN_DATA_SOURCE_H
