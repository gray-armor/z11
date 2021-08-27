#ifndef LIBZAZEN_LIBINPUT_DEVICE_H
#define LIBZAZEN_LIBINPUT_DEVICE_H

#include <libzazen.h>
#include <wayland-server.h>

struct udev_input {
  struct libinput *libinput;
};

void libinput_init(struct wl_event_loop *loop);

#endif  //  LIBZAZEN_LIBINPUT_DEVICE_H
