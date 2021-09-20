#ifndef LIBZAZEN_LIBINPUT_H
#define LIBZAZEN_LIBINPUT_H

#include <libinput.h>
#include <libudev.h>
#include <wayland-server.h>

#include "opengl_render_component_manager.h"

struct zazen_libinput {
  struct libinput *libinput;
  struct udev *udev;
  struct zazen_seat *seat;
};

struct zazen_libinput *zazen_libinput_create(struct zazen_seat *seat,
                                             struct wl_display *display);

void zazen_libinput_destroy(struct zazen_libinput *libinput);

#endif  //  LIBZAZEN_LIBINPUT_H
