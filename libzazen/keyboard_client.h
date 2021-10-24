#ifndef LIBZAZEN_KEYBOARD_CLIENT_H
#define LIBZAZEN_KEYBOARD_CLIENT_H

#include <wayland-server.h>

#include "keyboard.h"

struct zazen_keyboard_client {
  struct wl_list link;
  struct wl_client* client;
  struct wl_list keyboard_resources;

  struct wl_listener client_destroy_listener;
  struct wl_listener keyboard_destroy_signal_listener;
};

struct zazen_keyboard_client* zazen_keyboard_client_find_or_create(
    struct zazen_keyboard* keyboard, struct wl_client* client);

void zazen_keyboard_client_add_resource(
    struct zazen_keyboard_client* keyboard_client,
    struct wl_resource* resource);

#endif  // LIBZAZEN_KEYBOARD_CLIENT_H
