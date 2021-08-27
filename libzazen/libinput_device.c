#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

#include "libinput_device.h"

#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wayland-server-core.h>

#include "compositor.h"
#include "util.h"

static void print_event(struct libinput_event *event)
{
  struct libinput_device *device = libinput_event_get_device(event);
  const char *type = NULL;

  switch (libinput_event_get_type(event)) {
    case LIBINPUT_EVENT_NONE:
      abort();
    case LIBINPUT_EVENT_DEVICE_ADDED:
      type = "DEVICE_ADDED";
      break;
    case LIBINPUT_EVENT_DEVICE_REMOVED:
      type = "DEVICE_REMOVED";
      break;
    case LIBINPUT_EVENT_KEYBOARD_KEY:
      type = "KEYBOARD_KEY";
      break;
    case LIBINPUT_EVENT_POINTER_MOTION:
      type = "POINTER_MOTION";
      break;
    case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
      type = "POINTER_MOTION_ABSOLUTE";
      break;
    case LIBINPUT_EVENT_POINTER_BUTTON:
      type = "POINTER_BUTTON";
      break;
    case LIBINPUT_EVENT_POINTER_AXIS:
      type = "POINTER_AXIS";
      break;
    case LIBINPUT_EVENT_TOUCH_DOWN:
      type = "TOUCH_DOWN";
      break;
    case LIBINPUT_EVENT_TOUCH_MOTION:
      type = "TOUCH_MOTION";
      break;
    case LIBINPUT_EVENT_TOUCH_UP:
      type = "TOUCH_UP";
      break;
    case LIBINPUT_EVENT_TOUCH_CANCEL:
      type = "TOUCH_CANCEL";
      break;
    case LIBINPUT_EVENT_TOUCH_FRAME:
      type = "TOUCH_FRAME";
      break;
    case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
      type = "GESTURE_SWIPE_BEGIN";
      break;
    case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
      type = "GESTURE_SWIPE_UPDATE";
      break;
    case LIBINPUT_EVENT_GESTURE_SWIPE_END:
      type = "GESTURE_SWIPE_END";
      break;
    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
      type = "GESTURE_PINCH_BEGIN";
      break;
    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
      type = "GESTURE_PINCH_UPDATE";
      break;
    case LIBINPUT_EVENT_GESTURE_PINCH_END:
      type = "GESTURE_PINCH_END";
      break;
    case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
      type = "TABLET_TOOL_AXIS";
      break;
    case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
      type = "TABLET_TOOL_PROXIMITY";
      break;
    case LIBINPUT_EVENT_TABLET_TOOL_TIP:
      type = "TABLET_TOOL_TIP";
      break;
    case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
      type = "TABLET_TOOL_BUTTON";
      break;
    case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
      type = "TABLET_PAD_BUTTON";
      break;
    case LIBINPUT_EVENT_TABLET_PAD_RING:
      type = "TABLET_PAD_RING";
      break;
    case LIBINPUT_EVENT_TABLET_PAD_STRIP:
      type = "TABLET_PAD_STRIP";
      break;
    case LIBINPUT_EVENT_TABLET_PAD_KEY:
      type = "TABLET_PAD_KEY";
      break;
    case LIBINPUT_EVENT_SWITCH_TOGGLE:
      type = "SWITCH_TOGGLE";
      break;
  }

  zazen_log("%-7s  %-16s: %s\n", libinput_device_get_sysname(device), type, libinput_device_get_name(device));
}

static int libinput_handle_event(struct libinput *libinput)
{
  int rc = -1;
  struct libinput_event *event;

  libinput_dispatch(libinput);
  while ((event = libinput_get_event(libinput))) {
    print_event(event);

    libinput_event_destroy(event);
    rc = 0;
  }

  return rc;
}

static int libinput_source_dispatch(int fd, uint32_t mask, void *data)
{
  struct libinput *libinput = data;

  return libinput_handle_event(libinput) != 0;
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

void libinput_init(struct wl_event_loop *loop)
{
  struct udev_input *input;
  struct udev *udev;
  const char *seat;
  int fd;

  input = zalloc(sizeof(*input));

  udev = udev_new();

  seat = "seat0";

  if (!udev) {
    zazen_log("Failed to initialize udev\n");
    goto err_udev;
  }

  input->libinput = libinput_udev_create_context(&interface, input, udev);
  if (!input->libinput) {
    zazen_log("Failed to initialize context from udev\n");
    goto err_libinput;
  }

  if (libinput_udev_assign_seat(input->libinput, seat)) {
    zazen_log("Failed to set seat\n");
    goto err_libinput;
  }

  if (libinput_handle_event(input->libinput)) {
    zazen_log(
        "Expected device added events on startup but got none. "
        "Maybe you don't have the right permissions?\n");
    goto err_libinput;
  }

  fd = libinput_get_fd(input->libinput);
  wl_event_loop_add_fd(loop, fd, WL_EVENT_READABLE, libinput_source_dispatch, input->libinput);

  return;

err_libinput:
  libinput_unref(input->libinput);

err_udev:
  udev_unref(udev);
  free(input);
}

void libinput_destroy()
{
  libinput_unref(input->libinput);
  udev_unref(udev);
  free(input);
}
