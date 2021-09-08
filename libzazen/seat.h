#ifndef LIBZAZEN_SEAT_H
#define LIBZAZEN_SEAT_H

#include "keyboard.h"
#include "pointer.h"

struct zazen_seat {
  struct zazen_opengl_render_component_manager *render_component_manager;

  struct zazen_pointer *pointer;
  struct zazen_keyboard *keyboard;

  uint32_t pointer_device_count;
  uint32_t keyboard_device_count;

  char *seat_name;
};

bool zazen_seat_init_keyboard(struct zazen_seat *seat);

bool zazen_seat_init_pointer(struct zazen_seat *seat);

struct zazen_seat *zazen_seat_create(struct zazen_opengl_render_component_manager *render_component_manager,
                                     const char *seat_name);

void zazen_seat_destroy(struct zazen_seat *seat);

#endif  // LIBZAZEN_SEAT_H
