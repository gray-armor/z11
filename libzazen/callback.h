#ifndef LIBZAZEN_CALLBACK_H
#define LIBZAZEN_CALLBACK_H

#include <wayland-server.h>

struct zazen_callback {
  struct wl_resource *resource;
  struct wl_list link;
};

struct zazen_callback *zazen_callback_create(struct wl_client *client, uint32_t id);

void zazen_callback_done_with_current_time(struct zazen_callback *callback);

#endif  //  LIBZAZEN_CALLBACK_H
