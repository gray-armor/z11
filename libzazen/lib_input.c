#include "lib_input.h"

#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <string.h>
#include <unistd.h>

#include "seat.h"
#include "util.h"

static void handle_device_added(struct zazen_seat *seat,
                                struct libinput_device *device)
{
  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
    if (!zazen_seat_init_keyboard(seat)) {
      zazen_log("Failed to init keyboard\n");
    }
    return;
  }

  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
    if (!zazen_seat_init_ray(seat)) {
      zazen_log("Failed to init ray\n");
    }
  }
}

static void handle_pointer_motion(struct zazen_seat *seat,
                                  struct libinput_event_pointer *pointer_event)
{
  struct zazen_pointer_motion_event event = {0};
  double dx_unaccel, dy_unaccel;

  dx_unaccel = libinput_event_pointer_get_dx_unaccelerated(pointer_event);
  dy_unaccel = libinput_event_pointer_get_dy_unaccelerated(pointer_event);

  event = (struct zazen_pointer_motion_event){
      .mask = ZAZEN_POINTER_MOTION_REL | ZAZEN_POINTER_MOTION_REL_UNACCEL,
      .dx = libinput_event_pointer_get_dx(pointer_event),
      .dy = libinput_event_pointer_get_dy(pointer_event),
      .dx_unaccel = dx_unaccel,
      .dy_unaccel = dy_unaccel,
  };

  zazen_ray_notify_pointer_motion(seat->ray, &event);
}

static int handle_event(int fd, uint32_t mask, void *data)
{
  UNUSED(fd);
  UNUSED(mask);
  struct zazen_libinput *libinput;
  struct libinput_event *event;
  int result = -1;

  libinput = data;

  libinput_dispatch(libinput->libinput);
  while ((event = libinput_get_event(libinput->libinput))) {
    switch (libinput_event_get_type(event)) {
      case LIBINPUT_EVENT_POINTER_MOTION:
        handle_pointer_motion(libinput->seat,
                              libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_DEVICE_ADDED:
        handle_device_added(libinput->seat, libinput_event_get_device(event));
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        break;
      default:
        break;
    }
    libinput_event_destroy(event);
    result = 0;
  }

  return result;
}

static int open_restricted(const char *path, int flags, void *user_data)
{
  UNUSED(user_data);
  int fd = open(path, flags);

  if (fd < 0) zazen_log("Failed to open %s (%s)\n", path, strerror(errno));

  return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data)
{
  UNUSED(user_data);
  close(fd);
}

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

struct zazen_libinput *zazen_libinput_create(
    struct wl_display *display,
    struct zazen_opengl_render_component_manager *render_component_manager)
{
  struct zazen_libinput *libinput;
  int fd;

  libinput = zalloc(sizeof(*libinput));

  libinput->seat =
      zazen_seat_create(display, render_component_manager, "seat0");
  if (!libinput->seat) {
    zazen_log("Failed to create seat\n");
    goto out;
  }

  libinput->udev = udev_new();
  if (!libinput->udev) {
    zazen_log("Failed to initialize udev\n");
    goto out;
  }

  libinput->libinput =
      libinput_udev_create_context(&interface, libinput, libinput->udev);
  if (!libinput->libinput) {
    zazen_log("Failed to initialize context from udev\n");
    goto out;
  }

  if (libinput_udev_assign_seat(libinput->libinput,
                                libinput->seat->seat_name)) {
    zazen_log("Failed to set seat\n");
    goto out;
  }

  fd = libinput_get_fd(libinput->libinput);
  wl_event_loop_add_fd(wl_display_get_event_loop(display), fd,
                       WL_EVENT_READABLE, handle_event, libinput);

  return libinput;

out:
  if (libinput->seat) zazen_seat_destroy(libinput->seat);
  if (libinput->libinput) libinput_unref(libinput->libinput);
  if (libinput->udev) udev_unref(libinput->udev);
  free(libinput);

  return NULL;
}

void zazen_libinput_destroy(struct zazen_libinput *libinput)
{
  libinput_unref(libinput->libinput);
  udev_unref(libinput->udev);
  zazen_seat_destroy(libinput->seat);
  free(libinput);
}
