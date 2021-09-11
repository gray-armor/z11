#ifndef LIBZAZEN_RAY_H
#define LIBZAZEN_RAY_H

#include "opengl_render_item.h"

enum zazen_pointer_motion_mask {
  ZAZEN_POINTER_MOTION_ABS = 1 << 0,
  ZAZEN_POINTER_MOTION_REL = 1 << 1,
  ZAZEN_POINTER_MOTION_REL_UNACCEL = 1 << 2,
};

struct zazen_pointer_motion_event {
  enum zazen_pointer_motion_mask mask;
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

struct zazen_ray_grab;
struct zazen_ray_grab_interface {
  void (*focus)(struct zazen_ray_grab *grab);
  void (*motion)(struct zazen_ray_grab *grab,
                 struct zazen_pointer_motion_event *event);
  void (*button)(struct zazen_ray_grab *grab, const struct timespec *time,
                 uint32_t button, uint32_t state);
  void (*axis)(struct zazen_ray_grab *grab, const struct timespec *time,
               struct zazen_pointer_axis_event *event);
  void (*axis_source)(struct zazen_ray_grab *grab, uint32_t source);
  void (*frame)(struct zazen_ray_grab *grab);
  void (*cancel)(struct zazen_ray_grab *grab);
};

struct zazen_ray_grab {
  const struct zazen_ray_grab_interface *interface;
  struct zazen_ray *ray;
};

struct zazen_ray {
  struct zazen_seat *seat;
  struct zazen_ray_grab *grab;
  struct zazen_ray_grab default_grab;

  struct zazen_opengl_render_item *render_item;
};

void zazen_ray_notify_pointer_motion(struct zazen_ray *ray,
                                     struct zazen_pointer_motion_event *event);

struct zazen_ray *zazen_ray_create(struct zazen_seat *seat);

void zazen_ray_destroy(struct zazen_ray *zazen_ray);

#endif  // LIBZAZEN_RAY_H
