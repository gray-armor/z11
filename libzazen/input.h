#ifndef LIBZAZEN_INPUT_H
#define LIBZAZEN_INPUT_H

#include <wayland-server-core.h>

#include "compositor.h"
#include "libinput_device.h"
#include "opengl_render_component_manager.h"
#include "opengl_render_item.h"

enum zazen_pointer_motion_mask {
  ZAZEN_POINTER_MOTION_ABS = 1 << 0,
  ZAZEN_POINTER_MOTION_REL = 1 << 1,
  ZAZEN_POINTER_MOTION_REL_UNACCEL = 1 << 2,
};

struct zazen_pointer_motion_event {
  uint32_t mask;
  double x;
  double y;
  double dx;
  double dy;
  double dx_unaccel;
  double dy_unaccel;
};

struct zazen_pointer_axis_event {
  uint32_t axis;
  double value;
  bool has_discrete;
  int32_t discrete;
};

struct zazen_pointer_grab;
struct zazen_pointer_grab_interface {
  void (*focus)(struct zazen_pointer_grab *grab);
  void (*motion)(struct zazen_pointer_grab *grab, struct zazen_pointer_motion_event *event);
  void (*button)(struct zazen_pointer_grab *grab, const struct timespec *time, uint32_t button,
                 uint32_t state);
  void (*axis)(struct zazen_pointer_grab *grab, const struct timespec *time,
               struct zazen_pointer_axis_event *event);
  void (*axis_source)(struct zazen_pointer_grab *grab, uint32_t source);
  void (*frame)(struct zazen_pointer_grab *grab);
  void (*cancel)(struct zazen_pointer_grab *grab);
};

struct zazen_pointer_grab {
  const struct zazen_pointer_grab_interface *interface;
  struct zazen_pointer *pointer;
};

struct zazen_pointer {
  struct zazen_seat *seat;
  struct zazen_pointer_grab *grab;
  struct zazen_pointer_grab default_grab;

  struct zazen_opengl_render_item *render_item;
};

struct zazen_keyboard_grab;
struct zazen_keyboard_grab_interface {
  void (*key)(struct zazen_keyboard_grab *grab, const struct timespec *time, uint32_t key, uint32_t state);
  void (*modifiers)(struct zazen_keyboard_grab *grab, uint32_t serial, uint32_t mods_depressed,
                    uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
  void (*cancel)(struct zazen_keyboard_grab *grab);
};

struct zazen_keyboard_grab {
  const struct zazen_keyboard_grab_interface *interface;
  struct zazen_keyboard *keyboard;
};

struct zazen_keyboard {
  struct zazen_seat *seat;
  struct zazen_pointer_grab *grab;
  struct zazen_pointer_grab default_grab;

  struct zazen_opengl_render_item *render_item;
};

struct zazen_vr_controller {
  struct zazen_seat *seat;
  // TODO Handle vr controller input
};

struct zazen_seat {
  struct zazen_opengl_render_component_manager *render_component_manager;

  struct zazen_pointer *pointer;
  struct zazen_keyboard *keyboard;
  struct zazen_vr_controller *vr_controller;

  uint32_t pointer_device_count;
  uint32_t keyboard_device_count;

  char *seat_name;
};

void notify_motion(struct zazen_seat *seat, struct zazen_pointer_motion_event *event);

bool zazen_seat_init_keyboard(struct zazen_seat *seat);

bool zazen_seat_init_pointer(struct zazen_seat *seat);

bool zazen_seat_init(struct zazen_seat *seat,
                     struct zazen_opengl_render_component_manager *render_component_manager,
                     const char *seat_name);

struct zazen_input {
  struct zazen_seat *seat;
  struct udev_input *input;
};

struct zazen_input *zazen_input_create(
    struct wl_event_loop *loop, struct zazen_opengl_render_component_manager *render_component_manager);

void zazen_input_destroy(struct zazen_input *input);

#endif  //  LIBZAZEN_INPUT_H
