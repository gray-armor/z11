#ifndef LIBZAZEN_LIBINPUT_DEVICE_H
#define LIBZAZEN_LIBINPUT_DEVICE_H

#include <libinput.h>
#include <libudev.h>
#include <libzazen.h>
#include <wayland-server.h>

#include "input.h"
#include "opengl_render_item.h"

struct udev_input {
  struct libinput *libinput;
  struct udev *udev;
  struct zazen_seat *seat;
};

void libinput_init(struct wl_event_loop *loop, struct zazen_input *input_backend);

void libinput_destroy(struct zazen_input *input_backend);

#endif  //  LIBZAZEN_LIBINPUT_DEVICE_H
