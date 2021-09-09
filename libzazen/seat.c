#include "seat.h"

#include <string.h>

#include "util.h"

bool zazen_seat_init_keyboard(struct zazen_seat* seat)
{
  UNUSED(seat);
  // TODO: init keyboard
  return true;
}

bool zazen_seat_init_pointer(struct zazen_seat* seat)
{
  struct zazen_pointer* pointer;

  // TODO: Handle capabilities

  if (seat->pointer) {
    seat->pointer_device_count += 1;
    return true;
  }

  pointer = zazen_pointer_create(seat);
  if (pointer == NULL) {
    zazen_log("Unable to create pointer\n");
    return false;
  }

  seat->pointer = pointer;
  seat->pointer_device_count = 1;
  pointer->seat = seat;

  return true;
}

struct zazen_seat* zazen_seat_create(struct zazen_opengl_render_component_manager* render_component_manager,
                                     const char* seat_name)
{
  struct zazen_seat* seat;

  seat = zalloc(sizeof *seat);

  seat->render_component_manager = render_component_manager;

  seat->pointer = NULL;
  seat->keyboard = NULL;

  seat->pointer_device_count = 0;
  seat->keyboard_device_count = 0;

  seat->seat_name = strdup(seat_name);

  return seat;
}

void zazen_seat_destroy(struct zazen_seat* seat)
{
  if (seat->pointer) zazen_pointer_destroy(seat->pointer);
  if (seat->keyboard) zazen_keyboard_destroy(seat->keyboard);
  free(seat->seat_name);
  free(seat);
}
