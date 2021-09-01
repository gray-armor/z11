#ifndef LIBZAZEN_LIBINPUT_DEVICE_H
#define LIBZAZEN_LIBINPUT_DEVICE_H

#include <libzazen.h>
#include <wayland-server.h>

#include "opengl_for_render_state.h"

struct udev_input {
  struct libinput *libinput;
  struct zazen_opengl_render_item *render_item;
};

void libinput_init(struct wl_event_loop *loop, struct zazen_opengl_render_item *render_item);

void libinput_destroy();

#endif  //  LIBZAZEN_LIBINPUT_DEVICE_H
