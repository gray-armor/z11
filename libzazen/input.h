#ifndef LIBZAZEN_INPUT_H
#define LIBZAZEN_INPUT_H

#include "lib_input.h"
#include "opengl_render_component_manager.h"

struct zazen_input {
  struct zazen_libinput *libinput;
};

struct zazen_input *zazen_input_create(
    struct wl_event_loop *loop,
    struct zazen_opengl_render_component_manager *render_component_manager);

void zazen_input_destroy(struct zazen_input *input);

#endif  // LIBZAZEN_INPUT_H
