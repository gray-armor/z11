#ifndef LIBZAZEN_LIBINPUT_DEVICE_H
#define LIBZAZEN_LIBINPUT_DEVICE_H

#include <libinput.h>
#include <libzazen.h>
#include <wayland-server.h>

#include "input.h"
#include "opengl_item.h"

struct udev_input {
  struct libinput *libinput;
  struct zazen_seat *seat;
};

void libinput_init(struct wl_event_loop *loop, struct zazen_seat *seat);

void libinput_destroy();

#endif  //  LIBZAZEN_LIBINPUT_DEVICE_H
