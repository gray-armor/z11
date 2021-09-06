#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

#include "libinput_device.h"

#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <wayland-server-core.h>

#include "compositor.h"
#include "input.h"
#include "math.h"
#include "opengl_render_item.h"
#include "util.h"

static void handle_device_added(struct zazen_seat *seat, struct libinput_device *device)
{
  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
    if (!zazen_seat_init_keyboard(seat)) {
      zazen_log("Failed to init keyboard\n");
    }
    return;
  }

  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
    if (!zazen_seat_init_pointer(seat)) {
      zazen_log("Failed to init pointer\n");
    }
  }
}

static void handle_pointer_motion(struct zazen_seat *seat, struct libinput_event_pointer *pointer_event)
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

  notify_motion(seat, &event);
}

static int handle_event(struct udev_input *input)
{
  int rc = -1;
  struct libinput_event *event;

  libinput_dispatch(input->libinput);
  while ((event = libinput_get_event(input->libinput))) {
    switch (libinput_event_get_type(event)) {
      case LIBINPUT_EVENT_POINTER_MOTION:
        handle_pointer_motion(input->seat, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_DEVICE_ADDED:
        handle_device_added(input->seat, libinput_event_get_device(event));
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        break;
      default:
        break;
    }
    libinput_event_destroy(event);
    rc = 0;
  }

  return rc;
}

static int libinput_source_dispatch(int fd, uint32_t mask, void *data)
{
  struct udev_input *input = data;

  return handle_event(input) != 0;
}

static int open_restricted(const char *path, int flags, void *user_data)
{
  bool *grab = user_data;
  int fd = open(path, flags);

  if (fd < 0) zazen_log("Failed to open %s (%s)\n", path, strerror(errno));

  return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data) { close(fd); }

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

void libinput_init(struct wl_event_loop *loop, struct zazen_input *input_backend)
{
  struct udev_input *input;
  int fd;

  input = zalloc(sizeof(*input));

  input->seat = input_backend->seat;

  input->udev = udev_new();

  if (!input->udev) {
    zazen_log("Failed to initialize udev\n");
    goto err_udev;
  }

  input->libinput = libinput_udev_create_context(&interface, input, input->udev);
  if (!input->libinput) {
    zazen_log("Failed to initialize context from udev\n");
    goto err_libinput;
  }

  if (libinput_udev_assign_seat(input->libinput, input_backend->seat->seat_name)) {
    zazen_log("Failed to set seat\n");
    goto err_libinput;
  }

  if (handle_event(input)) {
    zazen_log(
        "Expected device added events on startup but got none. "
        "Maybe you don't have the right permissions?\n");
    goto err_libinput;
  }

  fd = libinput_get_fd(input->libinput);
  wl_event_loop_add_fd(loop, fd, WL_EVENT_READABLE, libinput_source_dispatch, input);

  input_backend->input = input;

  return;

err_libinput:
  libinput_unref(input->libinput);

err_udev:
  udev_unref(input->udev);
  free(input);
}

void libinput_destroy(struct zazen_input *input_backend)
{
  libinput_unref(input_backend->input->libinput);
  udev_unref(input_backend->input->udev);
  free(input_backend->input);
}
