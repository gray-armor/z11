#ifndef LIBZAZEN_RAY_H
#define LIBZAZEN_RAY_H

#include <cglm/cglm.h>
#include <libzazen.h>
#include <time.h>
#include <wayland-server.h>

#include "opengl_render_item.h"

typedef struct {
  vec3 origin, target;
} HalfLine;

struct zazen_ray_motion_event {
  vec3 begin_delta;
  vec3 end_delta;
};

struct zazen_ray_axis_event {
  uint32_t axis;
  double value;
  bool has_discrete;
  int32_t discrete;
};

struct zazen_ray_grab;
struct zazen_ray_grab_interface {
  void (*focus)(struct zazen_ray_grab *grab);
  void (*motion)(struct zazen_ray_grab *grab, const struct timespec *time,
                 struct zazen_ray_motion_event *event);
  void (*button)(struct zazen_ray_grab *grab, const struct timespec *time,
                 uint32_t button, uint32_t state);
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

  struct wl_list ray_clients;
  struct wl_signal destroy_signal;

  HalfLine line;

  struct zazen_opengl_render_item *render_item;
  struct zazen_cuboid_window *focus_cuboid_window;  // nullable
  struct wl_listener focus_cuboid_window_destroy_listener;
};

struct zazen_ray_client *zazen_ray_find_ray_client(struct zazen_ray *ray,
                                                   struct wl_client *client);

void zazen_ray_notify_motion(struct zazen_ray *ray, const struct timespec *time,
                             struct zazen_ray_motion_event *event);

void zazen_ray_notify_button(struct zazen_ray *ray, const struct timespec *time,
                             int32_t button,
                             enum wl_pointer_button_state state);

struct zazen_ray *zazen_ray_create(struct zazen_seat *seat);

void zazen_ray_destroy(struct zazen_ray *ray);

#endif  // LIBZAZEN_RAY_H
