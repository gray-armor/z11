#ifndef LIBZAZEN_SHELL_H
#define LIBZAZEN_SHELL_H

#include <wayland-server.h>

#include "opengl_render_component_manager.h"

struct zazen_shell {
  struct zazen_opengl_render_component_manager *render_component_manager;
  struct wl_list cuboid_window_list;
};

#endif  //  LIBZAZEN_SHELL_H
