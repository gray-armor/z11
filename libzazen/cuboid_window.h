#ifndef LIBZAZEN_CUBOID_WINDOW
#define LIBZAZEN_CUBOID_WINDOW

#include <cglm/cglm.h>
#include <libzazen.h>
#include <wayland-server.h>

#include "cuboid_window_back_state.h"
#include "opengl_render_component_manager.h"
#include "opengl_render_item.h"
#include "shell.h"
#include "virtual_object.h"

struct zazen_cuboid_window {
  float width;
  float height;
  float depth;
  struct wl_resource* resource;
  struct zazen_virtual_object* virtual_object;
  struct wl_listener virtual_object_destroy_listener;
  struct zazen_opengl_render_item* render_item;
  vec3 vertex_buffer[48];
  struct wl_listener virtual_object_model_matrix_change_listener;
  struct zazen_cuboid_window_back_state* back_state;
  struct wl_signal destroy_signal;
};

struct zazen_cuboid_window* zazen_cuboid_window_create(
    struct wl_client* client, uint32_t id,
    struct zazen_virtual_object* virtual_object, struct zazen_shell* shell,
    struct zazen_opengl_render_component_manager* manager);

void zazen_cuboid_window_highlight(struct zazen_cuboid_window* cuboid_window);

void zazen_cuboid_window_remove_highlight(
    struct zazen_cuboid_window* cuboid_window);

#endif  //  LIBZAZEN_CUBOID_WINDOW
