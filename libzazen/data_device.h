#ifndef LIBZAZEN_DATA_DEVICE_H
#define LIBZAZEN_DATA_DEVICE_H

#include <wayland-server.h>

#include "seat.h"

struct zazen_data_device {
  struct zazen_seat* seat;
  struct wl_client* client;
  struct wl_list data_device_resources;

  struct wl_listener client_destroy_listener;
};

struct zazen_data_device* zazen_data_device_find_or_create(
    struct zazen_seat* seat, struct wl_client* client);

void zazen_data_device_add_resource(struct zazen_data_device* data_device,
                                    struct wl_resource* resource);

#endif  // LIBZAZEN_DATA_DEVICE_H
