#ifndef LIBZAZEN_KEYBOARD_CLIENT_H
#define LIBZAZEN_KEYBOARD_CLIENT_H

#include <wayland-server.h>

#include "keyboard.h"

struct zazen_keyboard_client {
  struct wl_list link;
  struct wl_resource* resource;

  struct wl_listener keyboard_destroy_signal_listener;
};

struct zazen_keyboard_client* zazen_keyboard_client_create(
    struct zazen_keyboard* keyboard, struct wl_client* client, uint32_t id);

#endif  // LIBZAZEN_KEYBOARD_CLIENT_H
