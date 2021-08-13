#include "compositor.h"

#include "util.h"
#include "virtual_object.h"
#include "z11-server-protocol.h"

static void zazen_compositor_protocol_create_virtual_object(struct wl_client* client,
                                                            struct wl_resource* resource, uint32_t id)
{
  UNUSED(resource);
  struct zazen_virtual_object* virtual_object;

  virtual_object = zazen_virtual_object_create(client, id);
  if (virtual_object == NULL) {
    zazen_log("Failed to create a virtual object\n");
  }
}

static const struct z11_compositor_interface zazen_compositor_interface = {
    .create_virtual_object = zazen_compositor_protocol_create_virtual_object,
};

static void zazen_compositor_bind(struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zazen_compositor* compositor = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &z11_compositor_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &zazen_compositor_interface, compositor, NULL);
}

struct zazen_compositor* zazen_compositor_create(struct wl_display* display)
{
  struct zazen_compositor* compositor;

  compositor = zalloc(sizeof *compositor);
  if (compositor == NULL) goto out;

  if (wl_global_create(display, &z11_compositor_interface, 1, compositor, zazen_compositor_bind) == NULL)
    goto out;

  return compositor;

out:
  return NULL;
}
