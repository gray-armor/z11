#ifndef LIBZAZEN_INPUT_H
#define LIBZAZEN_INPUT_H

#include <wayland-server-core.h>

#include "compositor.h"
#include "opengl_render_component_manager.h"

int zazen_input_init(struct wl_event_loop* loop,
                     struct zazen_opengl_render_component_manager* render_component_manager);

void zazen_input_destroy();

#endif  //  LIBZAZEN_INPUT_H
