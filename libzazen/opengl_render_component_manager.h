#ifndef LIBZAZEN_OPENGL_RENDER_COMPONENT_MANAGER
#define LIBZAZEN_OPENGL_RENDER_COMPONENT_MANAGER

#include <wayland-server.h>

struct zazen_opengl_render_component_manager {
  struct wl_list render_component_back_state_list;
};

#endif  //  LIBZAZEN_OPENGL_RENDER_COMPONENT_MANAGER
