#ifndef LIBZAZEN_SEAT_H
#define LIBZAZEN_SEAT_H

#include "keyboard.h"
#include "keyboard_client.h"
#include "lib_input.h"
#include "ray.h"
#include "ray_client.h"

struct zazen_seat {
  struct zazen_opengl_render_component_manager *render_component_manager;
  struct wl_display *display;

  struct wl_list client_list;

  struct zazen_ray *ray;
  struct zazen_ray_back_state previous_ray_back_state;
  struct zazen_keyboard *keyboard;

  uint32_t ray_device_count;
  uint32_t keyboard_device_count;

  char *seat_name;

  struct zazen_libinput *libinput;
};

bool zazen_seat_init_ray(struct zazen_seat *seat);

bool zazen_seat_init_keyboard(struct zazen_seat *seat);

void zazen_seat_release_ray(struct zazen_seat *seat);

void zazen_seat_release_keyboard(struct zazen_seat *seat);

struct zazen_seat *zazen_seat_create(
    struct wl_display *display,
    struct zazen_opengl_render_component_manager *render_component_manager);

void zazen_seat_destroy(struct zazen_seat *seat);

#endif  // LIBZAZEN_SEAT_H
