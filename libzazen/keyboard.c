#include "keyboard.h"

#include "util.h"

static void grab_keyboard_key(struct zazen_keyboard_grab* grab,
                              const struct timespec* time, uint32_t key,
                              uint32_t state)
{
  UNUSED(grab);
  UNUSED(time);
  UNUSED(key);
  UNUSED(state);
  // TODO
}

static void grab_keyboard_modifiers(struct zazen_keyboard_grab* grab,
                                    uint32_t serial, uint32_t mods_depressed,
                                    uint32_t mods_latched, uint32_t mods_locked,
                                    uint32_t group)
{
  UNUSED(grab);
  UNUSED(serial);
  UNUSED(mods_depressed);
  UNUSED(mods_latched);
  UNUSED(mods_locked);
  UNUSED(group);
  // TODO
}

void grab_keyboard_cancel(struct zazen_keyboard_grab* grab)
{
  UNUSED(grab);
  // TODO
}

static const struct zazen_keyboard_grab_interface keyboard_grab_interface = {
    .key = grab_keyboard_key,
    .modifiers = grab_keyboard_modifiers,
    .cancel = grab_keyboard_cancel,
};

struct zazen_keyboard* zazen_keyboard_create(struct zazen_seat* seat)
{
  struct zazen_keyboard* keyboard;

  keyboard = zalloc(sizeof *keyboard);
  if (keyboard == NULL) return NULL;

  keyboard->seat = seat;

  keyboard->grab.interface = &keyboard_grab_interface;
  keyboard->grab.keyboard = keyboard;

  wl_list_init(&keyboard->keyboard_clients);
  wl_signal_init(&keyboard->destroy_signal);

  return keyboard;
}

void zazen_keyboard_destroy(struct zazen_keyboard* keyboard)
{
  wl_signal_emit(&keyboard->destroy_signal, keyboard);
  free(keyboard);
}
