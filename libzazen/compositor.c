#include <wayland-server.h>

#include "internal.h"
#include "z11-server-protocol.h"

struct zazen_compositor {
  struct wl_list render_element_list;
};

static void zazen_compositor_protocol_create_render_element(struct wl_client* client,
                                                            struct wl_resource* resource, uint32_t id)
{
  struct zazen_compositor* compositor = wl_resource_get_user_data(resource);
  struct zazen_render_element* render_element;

  render_element = zazen_render_element_create(client, id, compositor);
  if (render_element == NULL) {
    // TODO: error log
  }
}

static const struct z11_compositor_interface zazen_compositor_interface = {
    .create_render_element = zazen_compositor_protocol_create_render_element,
};

static void zazen_compositor_bind(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zazen_compositor* compositor = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_compositor_interface, version, id);
  if (resource == NULL) goto no_mem_resource;

  wl_resource_set_implementation(resource, &zazen_compositor_interface, compositor, NULL);

  return;

no_mem_resource:
  wl_client_post_no_memory(client);
}

void zazen_compositor_append_render_element(struct zazen_compositor* compositor,
                                            struct zazen_render_element* render_element)
{
  wl_list_insert(&compositor->render_element_list, zazen_render_element_get_link(render_element));
}

struct wl_list* zazen_compositor_get_render_element_list(struct zazen_compositor* compositor)
{
  return &compositor->render_element_list;
}

struct zazen_compositor* zazen_compositor_create(struct wl_display* display)
{
  struct zazen_compositor* compositor;

  compositor = zalloc(sizeof *compositor);
  if (compositor == NULL) goto fail;

  wl_list_init(&compositor->render_element_list);

  if (wl_global_create(display, &z11_compositor_interface, 1, compositor, zazen_compositor_bind) == NULL)
    goto fail;

  return compositor;

fail:
  return NULL;
}
