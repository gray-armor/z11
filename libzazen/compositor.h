#ifndef LIBZAZEN_COMPOSITOR_H
#define LIBZAZEN_COMPOSITOR_H

#include <cglm/cglm.h>
#include <wayland-server.h>

#include "cuboid_window.h"
#include "ray.h"
#include "seat.h"
#include "shell.h"

struct zazen_compositor {
  struct wl_signal frame_signal;
  struct zazen_shell* shell;
  struct zazen_seat* seat;
};

struct zazen_compositor* zazen_compositor_create(struct wl_display* display);

// return null when no cuboid window intersected
// local_ray_* and min_distance will be set only when intersected
struct zazen_cuboid_window* zazen_compositor_pick_cuboid_window(
    struct zazen_compositor* compositor, vec3 ray_origin, vec3 ray_direction,
    vec3 local_ray_origin, vec3 local_ray_direction, float* distance);

#endif  //  LIBZAZEN_COMPOSITOR_H
