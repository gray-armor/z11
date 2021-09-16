#include "cuboid_window.h"

#include <wayland-server.h>
#include <z11-server-protocol.h>

#include "util.h"

static void zazen_cuboid_window_destroy(
    struct zazen_cuboid_window* cuboid_window);

static void zazen_cuboid_window_handle_destroy(struct wl_resource* resource)
{
  struct zazen_cuboid_window* cuboid_window;

  cuboid_window = wl_resource_get_user_data(resource);

  zazen_cuboid_window_destroy(cuboid_window);
}

static void zazen_cuboid_window_protocol_request_window_size(
    struct wl_client* client, struct wl_resource* resource, wl_fixed_t width,
    wl_fixed_t height, wl_fixed_t depth)
{
  UNUSED(client);
  struct zazen_cuboid_window* cuboid_window;

  cuboid_window = wl_resource_get_user_data(resource);

  cuboid_window->width = width;
  cuboid_window->height = height;
  cuboid_window->depth = depth;

  z11_cuboid_window_send_configure(resource, width, height, depth);
}

static const struct z11_cuboid_window_interface zazen_cuboid_window_interface =
    {
        .request_window_size = zazen_cuboid_window_protocol_request_window_size,
};

struct zazen_cuboid_window* zazen_cuboid_window_create(struct wl_client* client,
                                                       uint32_t id)
{
  struct zazen_cuboid_window* cuboid_window;
  struct wl_resource* resource;

  cuboid_window = zalloc(sizeof *cuboid_window);
  if (cuboid_window == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  resource = wl_resource_create(client, &z11_cuboid_window_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_cuboid_window;
  }

  wl_resource_set_implementation(resource, &zazen_cuboid_window_interface,
                                 cuboid_window,
                                 zazen_cuboid_window_handle_destroy);

  return cuboid_window;

out_cuboid_window:
  free(cuboid_window);

out:
  return NULL;
}

static void zazen_cuboid_window_destroy(
    struct zazen_cuboid_window* cuboid_window)
{
  free(cuboid_window);
}
