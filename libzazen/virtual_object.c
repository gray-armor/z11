#include "virtual_object.h"

#include <wayland-server.h>

#include "util.h"
#include "z11-server-protocol.h"

static void zazen_virtual_object_destroy(struct zazen_virtual_object *virtual_object);

static void zazen_virtual_object_handle_destroy(struct wl_resource *resource)
{
  struct zazen_virtual_object *virtual_object;

  virtual_object = wl_resource_get_user_data(resource);

  zazen_virtual_object_destroy(virtual_object);
}

static void zazen_virtual_object_protocol_destroy(struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}
static void zazen_virtual_object_protocol_commit(struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  struct zazen_virtual_object *virtual_object;

  virtual_object = wl_resource_get_user_data(resource);

  wl_signal_emit(&virtual_object->commit_signal, virtual_object);
}
static void zazen_virtual_object_protocol_frame(struct wl_client *client, struct wl_resource *resource,
                                                uint32_t callback)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(callback);
  // TODO: implement
}

static const struct z11_virtual_object_interface zazen_virtual_object_interface = {
    .destroy = zazen_virtual_object_protocol_destroy,
    .commit = zazen_virtual_object_protocol_commit,
    .frame = zazen_virtual_object_protocol_frame,
};

struct zazen_virtual_object *zazen_virtual_object_create(struct wl_client *client, uint32_t id)
{
  struct zazen_virtual_object *virtual_object;
  struct wl_resource *resource;

  virtual_object = zalloc(sizeof *virtual_object);
  if (virtual_object == NULL) {
    wl_client_post_no_memory(client);
    goto out;
  }

  resource = wl_resource_create(client, &z11_virtual_object_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    goto out_virtual_object;
  }

  wl_resource_set_implementation(resource, &zazen_virtual_object_interface, virtual_object,
                                 zazen_virtual_object_handle_destroy);

  virtual_object->resource = resource;

  wl_signal_init(&virtual_object->destroy_signal);
  wl_signal_init(&virtual_object->commit_signal);

  return virtual_object;

out_virtual_object:
  free(virtual_object);

out:
  return NULL;
}

static void zazen_virtual_object_destroy(struct zazen_virtual_object *virtual_object)
{
  wl_signal_emit(&virtual_object->destroy_signal, virtual_object);
  free(virtual_object);
}
