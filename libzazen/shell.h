#ifndef LIBZAZEN_SHELL_H
#define LIBZAZEN_SHELL_H

#include "opengl_render_component_manager.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zazen_shell {
  struct zazen_opengl_render_component_manager *render_component_manager;

  // back states
  struct wl_list cuboid_window_back_state_list;
};
#pragma GCC diagnostic pop

#endif  //  LIBZAZEN_SHELL_H
