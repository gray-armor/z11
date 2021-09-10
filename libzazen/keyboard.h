#ifndef LIBZAZEN_KEYBOARD_H
#define LIBZAZEN_KEYBOARD_H

#include "opengl_render_item.h"

struct zazen_keyboard_grab;
struct zazen_keyboard_grab_interface {
  void (*key)(struct zazen_keyboard_grab *grab, const struct timespec *time,
              uint32_t key, uint32_t state);
  void (*modifiers)(struct zazen_keyboard_grab *grab, uint32_t serial,
                    uint32_t mods_depressed, uint32_t mods_latched,
                    uint32_t mods_locked, uint32_t group);
  void (*cancel)(struct zazen_keyboard_grab *grab);
};

struct zazen_keyboard_grab {
  const struct zazen_keyboard_grab_interface *interface;
  struct zazen_keyboard *keyboard;
};

struct zazen_keyboard {
  struct zazen_seat *seat;
  struct zazen_keyboard_grab *grab;
  struct zazen_keyboard_grab default_grab;

  struct zazen_opengl_render_item *render_item;
};

struct zazen_keyboard *zazen_keyboard_create(struct zazen_seat *seat);

void zazen_keyboard_destroy(struct zazen_keyboard *zazen_keyboard);

#endif  // LIBZAZEN_KEYBOARD_H
