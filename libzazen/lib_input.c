#include "lib_input.h"

#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <string.h>
#include <unistd.h>

#include "ray.h"
#include "seat.h"
#include "util.h"

static void handle_pointer_motion(struct zazen_seat *seat,
                                  struct libinput_event_pointer *pointer_event)
{
  float dx, dy;
  struct timespec time;
  dx = libinput_event_pointer_get_dx(pointer_event);
  dy = libinput_event_pointer_get_dy(pointer_event);

  struct zazen_ray_motion_event event = {
      .begin_delta = GLM_VEC3_ZERO_INIT,
      .end_delta = {dx / 10, -dy / 10, 0},
  };

  timespec_from_usec(&time,
                     libinput_event_pointer_get_time_usec(pointer_event));

  zazen_ray_notify_motion(seat->ray, &time, &event);
}

static void handle_pointer_button(struct zazen_seat *seat,
                                  struct libinput_event_pointer *event)
{
  int button_state = libinput_event_pointer_get_button_state(event);
  int seat_button_count = libinput_event_pointer_get_seat_button_count(event);

  if ((button_state == LIBINPUT_BUTTON_STATE_PRESSED &&
       seat_button_count != 1) ||
      (button_state == LIBINPUT_BUTTON_STATE_RELEASED &&
       seat_button_count != 0))
    return;

  uint64_t time_usec = libinput_event_pointer_get_time_usec(event);

  zazen_ray_notify_button(seat->ray, time_usec,
                          libinput_event_pointer_get_button(event),
                          button_state);
}

static void handle_keyboard_key(struct zazen_seat *seat,
                                struct libinput_event_keyboard *event)
{
  UNUSED(seat);
  int key_state = libinput_event_keyboard_get_key_state(event);
  int seat_key_count = libinput_event_keyboard_get_seat_key_count(event);

  if ((key_state == LIBINPUT_KEY_STATE_PRESSED && seat_key_count != 1) ||
      (key_state == LIBINPUT_KEY_STATE_RELEASED && seat_key_count != 0))
    return;

  uint64_t time_usec = libinput_event_keyboard_get_time_usec(event);

  // TODO: STATE_UPDATE_AUTOMATIC とは？
  zazen_keyboard_notify_key(seat->keyboard, time_usec,
                            libinput_event_keyboard_get_key(event), key_state);
}

static void handle_device_added(struct zazen_seat *seat,
                                struct libinput_device *device)
{
  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
    if (!zazen_seat_init_keyboard(seat)) {
      zazen_log("Failed to init keyboard\n");
    }
  }

  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
    if (!zazen_seat_init_ray(seat)) {
      zazen_log("Failed to init ray\n");
    }
  }
}

static void handle_device_removed(struct zazen_seat *seat,
                                  struct libinput_device *device)
{
  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD)) {
    zazen_seat_release_keyboard(seat);
  }

  if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER)) {
    zazen_seat_release_ray(seat);
  }
}

static void handle_pointer_button(struct zazen_seat *seat,
                                  struct libinput_event_pointer *pointer_event)
{
  struct timespec time;
  int button_state = libinput_event_pointer_get_button_state(pointer_event);
  int seat_button_count =
      libinput_event_pointer_get_seat_button_count(pointer_event);

  if ((button_state == LIBINPUT_BUTTON_STATE_PRESSED &&
       seat_button_count != 1) ||
      (button_state == LIBINPUT_BUTTON_STATE_RELEASED &&
       seat_button_count != 0))
    return;

  timespec_from_usec(&time,
                     libinput_event_pointer_get_time_usec(pointer_event));

  zazen_ray_notify_button(seat->ray, &time,
                          libinput_event_pointer_get_button(pointer_event),
                          button_state);
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
      case LIBINPUT_EVENT_POINTER_BUTTON:
        handle_pointer_button(libinput->seat,
                              libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_KEYBOARD_KEY:
        handle_keyboard_key(libinput->seat,
                            libinput_event_get_keyboard_event(event));
        break;
      case LIBINPUT_EVENT_DEVICE_ADDED:
        handle_device_added(libinput->seat, libinput_event_get_device(event));
        break;
      case LIBINPUT_EVENT_DEVICE_REMOVED:
        handle_device_removed(libinput->seat, libinput_event_get_device(event));
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

struct zazen_libinput *zazen_libinput_create(struct zazen_seat *seat,
                                             struct wl_display *display)
{
  struct zazen_libinput *libinput;
  int fd;

  libinput = zalloc(sizeof(*libinput));
  if (libinput == NULL) return NULL;

  libinput->seat = seat;

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
  if (libinput->libinput) libinput_unref(libinput->libinput);
  if (libinput->udev) udev_unref(libinput->udev);
  free(libinput);

  return NULL;
}

void zazen_libinput_destroy(struct zazen_libinput *libinput)
{
  libinput_unref(libinput->libinput);
  udev_unref(libinput->udev);
  free(libinput);
}
