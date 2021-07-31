#include <wayland-server.h>

#include "internal.h"
#include "z11-server-protocol.h"

struct z_compositor {
  struct wl_display* display;
  struct wl_list list;
};

static void z_compositor_protocol_create_render_block(struct wl_client* client, struct wl_resource* resource,
                                                      uint32_t id)
{
  struct z_compositor* compositor = wl_resource_get_user_data(resource);
  struct z_render_block* render_block;

  render_block = z_render_block_create(client, id, compositor);
  if (render_block == NULL) {
    // TODO: error log
  }
}

static const struct z11_compositor_interface z_compositor_interface = {
    .create_render_block = z_compositor_protocol_create_render_block,
};

static void z_compositor_bind(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct z_compositor* compositor = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_compositor_interface, version, id);
  if (resource == NULL) goto no_mem_resource;

  wl_resource_set_implementation(resource, &z_compositor_interface, compositor, NULL);

  return;

no_mem_resource:
  wl_client_post_no_memory(client);
}

void z_compositor_append_render_block(struct z_compositor* compositor, struct z_render_block* render_block)
{
  wl_list_insert(&compositor->list, z_render_block_get_link(render_block));
}

struct wl_list* z_compositor_get_render_block_list(struct z_compositor* compositor)
{
  return &compositor->list;
}

struct z_compositor* z_compositor_create(struct wl_display* display)
{
  struct z_compositor* compositor;

  compositor = zalloc(sizeof *compositor);
  if (compositor == NULL) goto fail;

  compositor->display = display;
  wl_list_init(&compositor->list);

  if (wl_global_create(display, &z11_compositor_interface, 1, compositor, z_compositor_bind) == NULL)
    goto fail;

  return compositor;

fail:
  return NULL;
}